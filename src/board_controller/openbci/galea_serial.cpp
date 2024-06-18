#include <fstream>
#include <iostream>
#include <numeric>
#include <stdint.h>
#include <string.h>

#include "custom_cast.h"
#include "galea_serial.h"
#include "timestamp.h"

#include "json.hpp"

using json = nlohmann::json;

#ifndef _WIN32
#include <errno.h>
#endif

#define START_BYTE 0xA0
#define END_BYTE 0xC0


GaleaSerial::GaleaSerial (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::GALEA_SERIAL_BOARD, params)
{
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    half_rtt = 0.0;
}

GaleaSerial::~GaleaSerial ()
{
    skip_logs = true;
    release_session ();
}

int GaleaSerial::prepare_session ()
{
    // check params
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "serial port is not specified.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if ((params.timeout > 600) || (params.timeout < 1))
    {
        params.timeout = 3;
    }

    // create serial object
    serial = Serial::create (params.serial_port.c_str (), this);
    int res = serial->open_serial_port ();
    if (res < 0)
    {
        safe_logger (spdlog::level::err,
            "Make sure you provided correct port name and have permissions to open it(run with "
            "sudo/admin). Also, close all other apps using this port.");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    res = serial->set_serial_port_settings (params.timeout * 1000, false);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set port settings, res is {}", res);
        delete serial;
        serial = NULL;
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    res = serial->set_custom_baudrate (921600);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set custom baud rate, res is {}", res);
        delete serial;
        serial = NULL;
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    safe_logger (spdlog::level::trace, "set port settings");

    // set initial settings
    std::string tmp;
    std::string default_settings = "o"; // use demo mode with agnd
    res = config_board (default_settings, tmp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to apply default settings");
        delete serial;
        serial = NULL;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    std::string sampl_rate = "~6";
    res = config_board (sampl_rate, tmp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to apply defaul sampling rate");
        delete serial;
        serial = NULL;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    initialized = true;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GaleaSerial::config_board (std::string conf, std::string &response)
{
    if (serial == NULL)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    // special handling for calc_time command
    if (conf == "calc_time")
    {
        if (is_streaming)
        {
            safe_logger (spdlog::level::err, "can not calc delay during the streaming.");
            return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
        }
        int res = calc_time (response);
        return res;
    }

    if (gain_tracker.apply_config (conf) == (int)OpenBCICommandTypes::INVALID_COMMAND)
    {
        safe_logger (spdlog::level::warn, "invalid command: {}", conf.c_str ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    std::string new_conf = conf + "\n";
    int len = (int)new_conf.size ();
    safe_logger (spdlog::level::debug, "Trying to config GaleaSerial with {}", new_conf.c_str ());
    int res = serial->send_to_serial_port (new_conf.c_str (), len);
    if (len != res)
    {
        safe_logger (spdlog::level::err, "Failed to config a board");
        gain_tracker.revert_config ();
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    if (is_streaming)
    {
        safe_logger (spdlog::level::warn,
            "reconfiguring device during the streaming may lead to inconsistent data, it's "
            "recommended to call stop_stream before config_board");
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GaleaSerial::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    // calc time before start stream
    std::string resp;
    for (int i = 0; i < 3; i++)
    {
        int res = calc_time (resp);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    // start streaming
    res = serial->send_to_serial_port ("b\n", 2);
    if (res != 2)
    {
        safe_logger (spdlog::level::err, "Failed to send a command to board");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 3 * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 5sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
}

int GaleaSerial::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        int res = serial->send_to_serial_port ("s\n", 2);
        if (res != 2)
        {
            safe_logger (spdlog::level::err, "Failed to send a command to board");
            return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
        }

        // free kernel buffer
        unsigned char b;
        res = 1;
        int max_attempt = 400000; // to dont get to infinite loop
        int current_attempt = 0;
        while (res == 1)
        {
            res = serial->read_from_serial_port (&b, 1);
            current_attempt++;
            if (current_attempt == max_attempt)
            {
                safe_logger (
                    spdlog::level::err, "Command 's' was sent but streaming is still running.");
                return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
            }
        }

        std::string resp;
        for (int i = 0; i < 3; i++)
        {
            res = calc_time (resp); // call it in the end once to print time in the end
            if (res != (int)BrainFlowExitCodes::STATUS_OK)
            {
                break; // dont send exit code
            }
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int GaleaSerial::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
        if (serial)
        {
            delete serial;
            serial = NULL;
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void GaleaSerial::read_thread ()
{
    int res = 0;
    constexpr int package_size = 72;
    constexpr int max_num_packages = 25;
    constexpr int max_transaction_size = package_size * max_num_packages + 2;
    unsigned char b[max_transaction_size];
    DataBuffer time_buffer (1, 11);
    double latest_times[10];
    for (int i = 0; i < max_transaction_size; i++)
    {
        b[i] = 0;
    }
    int num_exg_rows = board_descr["default"]["num_rows"];
    int num_aux_rows = board_descr["auxiliary"]["num_rows"];
    double *exg_package = new double[num_exg_rows];
    double *aux_package = new double[num_aux_rows];
    for (int i = 0; i < num_exg_rows; i++)
    {
        exg_package[i] = 0.0;
    }
    for (int i = 0; i < num_aux_rows; i++)
    {
        aux_package[i] = 0.0;
    }

    while (keep_alive)
    {
        // read and check first byte
        int res = serial->read_from_serial_port (b, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "unable to read 1 byte");
            continue;
        }
        if (b[0] != START_BYTE)
        {
            continue;
        }
        double pc_timestamp = get_timestamp ();
        // read and check reamining bytes
        int pos = 1;
        int num_packages = 0;
        while (keep_alive)
        {
            if (pos >= 2 + package_size)
            {
                if ((b[0] == START_BYTE) && (b[pos] == END_BYTE) && ((pos - 2) % package_size == 0))
                {
                    num_packages = (pos - 2) / package_size;
                    break;
                }
            }
            if (pos > max_transaction_size - 1)
            {
                num_packages = 0;
                break;
            }
            res = serial->read_from_serial_port (b + pos, 1);
            if (res > 0)
            {
                pos += res;
            }
        }
        if (!keep_alive)
        {
            break;
        }
        if (num_packages < 1)
        {
            if (pos >= 2)
                safe_logger (spdlog::level::warn,
                    "Failed to parse some data, b[0]: {}, b[pos]: {}, size: {}", (int)b[0],
                    (int)b[pos], (int)pos);
            continue;
        }
        else
        {
            if (this->state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::info, "received first package streaming is started");
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }
        }

        // calc delta between PC timestamp and device timestamp in last 10 packages,
        // use this delta later on to assign timestamps
        unsigned char *package_bytes = b + 1;
        int offset_last_package = package_size * (num_packages - 1);
        double timestamp_last_package = 0.0;
        memcpy (&timestamp_last_package, package_bytes + 64 + offset_last_package, 8);
        timestamp_last_package /= 1000; // from ms to seconds
        double time_delta = pc_timestamp - timestamp_last_package;
        time_buffer.add_data (&time_delta);
        int num_time_deltas = (int)time_buffer.get_current_data (10, latest_times);
        time_delta = 0.0;
        for (int i = 0; i < num_time_deltas; i++)
        {
            time_delta += latest_times[i];
        }
        time_delta /= num_time_deltas;

        for (int cur_package = 0; cur_package < num_packages; cur_package++)
        {
            int offset = cur_package * package_size;
            // exg(default preset)
            exg_package[board_descr["default"]["package_num_channel"].get<int> ()] =
                (double)package_bytes[0 + offset];
            for (int i = 4, tmp_counter = 0; i < 20; i++, tmp_counter++)
            {
                double exg_scale = (double)(4.5 / float ((pow (2, 23) - 1)) /
                    gain_tracker.get_gain_for_channel (tmp_counter) * 1000000.);
                exg_package[i - 3] = exg_scale *
                    (double)cast_24bit_to_int32 (package_bytes + offset + 5 + 3 * (i - 4));
            }
            double timestamp_device = 0.0;
            memcpy (&timestamp_device, package_bytes + 64 + offset, 8);
            timestamp_device /= 1000; // from ms to seconds
            exg_package[board_descr["default"]["timestamp_channel"].get<int> ()] =
                timestamp_device + time_delta - half_rtt;
            exg_package[board_descr["default"]["other_channels"][0].get<int> ()] = pc_timestamp;
            exg_package[board_descr["default"]["other_channels"][1].get<int> ()] = timestamp_device;

            push_package (exg_package);

            // aux, 5 times slower
            if (((int)package_bytes[0 + offset]) % 5 == 0)
            {
                aux_package[board_descr["auxiliary"]["package_num_channel"].get<int> ()] =
                    (double)package_bytes[0 + offset];
                uint16_t temperature = 0;
                int32_t ppg_ir = 0;
                int32_t ppg_red = 0;
                float eda;
                memcpy (&temperature, package_bytes + 54 + offset, 2);
                memcpy (&eda, package_bytes + 1 + offset, 4);
                memcpy (&ppg_red, package_bytes + 56 + offset, 4);
                memcpy (&ppg_ir, package_bytes + 60 + offset, 4);
                // ppg
                aux_package[board_descr["auxiliary"]["ppg_channels"][0].get<int> ()] =
                    (double)ppg_red;
                aux_package[board_descr["auxiliary"]["ppg_channels"][1].get<int> ()] =
                    (double)ppg_ir;
                // eda
                aux_package[board_descr["auxiliary"]["eda_channels"][0].get<int> ()] = (double)eda;
                // temperature
                aux_package[board_descr["auxiliary"]["temperature_channels"][0].get<int> ()] =
                    temperature / 100.0;
                // battery
                aux_package[board_descr["auxiliary"]["battery_channel"].get<int> ()] =
                    (double)package_bytes[53 + offset];
                aux_package[board_descr["auxiliary"]["timestamp_channel"].get<int> ()] =
                    timestamp_device + time_delta - half_rtt;
                aux_package[board_descr["auxiliary"]["other_channels"][0].get<int> ()] =
                    pc_timestamp;
                aux_package[board_descr["auxiliary"]["other_channels"][1].get<int> ()] =
                    timestamp_device;
                push_package (aux_package, (int)BrainFlowPresets::AUXILIARY_PRESET);
            }
        }
    }
    delete[] exg_package;
    delete[] aux_package;
}

int GaleaSerial::calc_time (std::string &resp)
{
    constexpr int bytes_to_calc_rtt = 8;
    unsigned char b[bytes_to_calc_rtt];

    double start = get_timestamp ();
    int res = serial->send_to_serial_port ("F4444444\n", 9);
    if (res != 9)
    {
        safe_logger (spdlog::level::warn, "failed to send time calc command to device");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    res = serial->read_from_serial_port (b, bytes_to_calc_rtt);
    double done = get_timestamp ();
    if (res != bytes_to_calc_rtt)
    {
        safe_logger (
            spdlog::level::warn, "failed to recv resp from time calc command, resp size {}", res);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    double duration = done - start;
    double timestamp_device = 0;
    memcpy (&timestamp_device, b, 8);
    timestamp_device /= 1000;
    half_rtt = duration / 2;

    json result;
    result["rtt"] = duration;
    result["timestamp_device"] = timestamp_device;
    result["pc_timestamp"] = start + half_rtt;

    resp = result.dump ();
    safe_logger (spdlog::level::info, "calc_time output: {}", resp);

    return (int)BrainFlowExitCodes::STATUS_OK;
}