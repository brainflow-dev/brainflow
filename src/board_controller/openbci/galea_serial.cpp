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

#define START_BYTE_EXG 0xA0
#define EXG_PACKAGE_SIZE 58
#define AUX_PACKAGE_SIZE 26
#define START_BYTE_AUX 0XA1
#define END_BYTE 0xC0

GaleaSerial::GaleaSerial (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::GALEA_SERIAL_BOARD, params)
{
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
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

    std::string new_conf = conf + "\n";
    int len = (int)new_conf.size ();
    safe_logger (spdlog::level::debug, "Trying to config GaleaSerial with {}", new_conf.c_str ());
    int res = serial->send_to_serial_port (new_conf.c_str (), len);
    if (len != res)
    {
        safe_logger (spdlog::level::err, "Failed to config a board");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
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
        int max_attempt = 40000; // to dont get to infinite loop
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
    int res;
    constexpr int max_bytes = 8192;
    unsigned char b[max_bytes];
    for (int i = 0; i < max_bytes; i++)
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
        res = serial->read_from_serial_port (b, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "unable to read 1 byte");
            continue;
        }
        double pc_timestamp = get_timestamp ();
        int remaining_bytes = 0;
        if (b[0] == START_BYTE_EXG)
        {
            remaining_bytes = EXG_PACKAGE_SIZE - 1;
        }
        else if (b[0] == START_BYTE_AUX)
        {
            remaining_bytes = AUX_PACKAGE_SIZE - 1;
        }
        else
        {
            continue;
        }
        // read and check reamining bytes
        int pos = 1;
        while ((remaining_bytes > 0) && (keep_alive))
        {
            res = serial->read_from_serial_port (b + pos, remaining_bytes);
            if (res > 0)
            {
                remaining_bytes -= res;
                pos += res;
            }
        }
        if (!keep_alive)
        {
            break;
        }
        if ((b[0] == START_BYTE_EXG) && (b[EXG_PACKAGE_SIZE - 1] != END_BYTE))
        {
            safe_logger (spdlog::level::warn, "Wrong end byte");
            b[EXG_PACKAGE_SIZE] = '\0';
            safe_logger (spdlog::level::trace, "Received: {}", b);
            continue;
        }
        else if ((b[0] == START_BYTE_AUX) && (b[AUX_PACKAGE_SIZE - 1] != END_BYTE))
        {
            safe_logger (spdlog::level::warn, "Wrong end byte");
            b[AUX_PACKAGE_SIZE] = '\0';
            safe_logger (spdlog::level::trace, "Received: {}", b);
            continue;
        }
        else
        {
            if (this->state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::trace, "received first package");
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }
        }
        if (b[0] == START_BYTE_EXG)
        {
            add_exg_package (exg_package, b, pc_timestamp);
        }
        if (b[0] == START_BYTE_AUX)
        {
            add_aux_package (exg_package, b, pc_timestamp);
        }
    }
    delete[] exg_package;
    delete[] aux_package;
}

void GaleaSerial::add_exg_package (double *package, unsigned char *b, double pc_timestamp)
{
    // b[0] start byte
    // b[1] package num
    // b[2-48] exg
    // b[49-56] timestamp
    // b[57] end byte
    package[board_descr["default"]["package_num_channel"].get<int> ()] = (double)b[1];
    for (int i = 0; i < 16; i++)
    {
        if (i < 6)
            package[i + 1] = emg_scale * (double)cast_24bit_to_int32 (b + 2 + 3 * i);
        else if ((i == 6) || (i == 7)) // fp1 and fp2
            package[i + 1] = eeg_scale_sister_board * (double)cast_24bit_to_int32 (b + 2 + 3 * i);
        else
            package[i + 1] = eeg_scale_main_board * (double)cast_24bit_to_int32 (b + 2 + 3 * i);
    }

    double timestamp_device = 0.0;
    memcpy (&timestamp_device, b + 49, 8);
    timestamp_device /= 1000; // from ms to seconds
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = pc_timestamp;
    package[board_descr["default"]["other_channels"][0].get<int> ()] = pc_timestamp;
    package[board_descr["default"]["other_channels"][1].get<int> ()] = timestamp_device;

    push_package (package);
}

void GaleaSerial::add_aux_package (double *package, unsigned char *b, double pc_timestamp)
{
    // b[0] start byte
    // b[1] package num
    // b[2-3] temperature
    // b[4-7] eda
    // b[8-15] ppg
    // b[16] battery
    // b[17-24] timestamp
    // b[25] end byte
    package[board_descr["auxiliary"]["package_num_channel"].get<int> ()] = (double)b[1];
    uint16_t temperature = 0;
    int32_t ppg_ir = 0;
    int32_t ppg_red = 0;
    float eda = 0;
    memcpy (&temperature, b + 2, 2);
    memcpy (&eda, b + 4, 4);
    memcpy (&ppg_red, b + 8, 4);
    memcpy (&ppg_ir, b + 12, 4);
    package[board_descr["auxiliary"]["ppg_channels"][0].get<int> ()] = (double)ppg_red;
    package[board_descr["auxiliary"]["ppg_channels"][1].get<int> ()] = (double)ppg_ir;
    package[board_descr["auxiliary"]["eda_channels"][0].get<int> ()] = (double)eda;
    package[board_descr["auxiliary"]["temperature_channels"][0].get<int> ()] = temperature / 100.0;
    package[board_descr["auxiliary"]["battery_channel"].get<int> ()] = (double)b[16];

    double timestamp_device = 0.0;
    memcpy (&timestamp_device, b + 17, 8);
    timestamp_device /= 1000; // from ms to seconds
    package[board_descr["auxiliary"]["timestamp_channel"].get<int> ()] = pc_timestamp;
    package[board_descr["auxiliary"]["other_channels"][0].get<int> ()] = pc_timestamp;
    package[board_descr["auxiliary"]["other_channels"][1].get<int> ()] = timestamp_device;

    push_package (package, (int)BrainFlowPresets::AUXILIARY_PRESET);
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

    json result;
    result["rtt"] = duration;
    result["timestamp_device"] = timestamp_device;
    result["pc_timestamp"] = start + duration / 2.0;

    resp = result.dump ();
    safe_logger (spdlog::level::info, "calc_time output: {}", resp);

    return (int)BrainFlowExitCodes::STATUS_OK;
}
