#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "ironbci.h"
#include "serial.h"
#include "timestamp.h"

constexpr int IronBCI::ads_gain;
constexpr int IronBCI::start_byte;
constexpr int IronBCI::stop_byte;
const std::string IronBCI::start_command = "b";
const std::string IronBCI::stop_command = "s";


IronBCI::IronBCI (struct BrainFlowInputParams params) : Board ((int)BoardIds::IRONBCI_BOARD, params)
{
    serial = NULL;
    keep_alive = false;
    initialized = false;
}

IronBCI::~IronBCI ()
{
    skip_logs = true;
    release_session ();
}

int IronBCI::config_board (std::string config, std::string &response)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::send_to_board (const char *msg)
{
    if ((!initialized) || (serial == NULL))
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int lenght = (int)strlen (msg);
    safe_logger (spdlog::level::debug, "Sending {} to the board", msg);
    int res = serial->send_to_serial_port ((const void *)msg, lenght);
    if (res != lenght)
    {
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "Serial port is empty");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    serial = Serial::create (params.serial_port.c_str (), this);

    safe_logger (spdlog::level::info, "Openning port {}", serial->get_port_name ());
    int res = serial->open_serial_port ();
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Open port error {}", res);
        delete serial;
        serial = NULL;
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    res = serial->set_serial_port_settings ();
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set port settings, res is {}", res);
        delete serial;
        serial = NULL;
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::start_stream (int buffer_size, char *streamer_params)
{
    if (keep_alive)
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
    int send_res = send_to_board (IronBCI::start_command.c_str ());
    if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return send_res;
    }
    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        if (streaming_thread.joinable ())
        {
            streaming_thread.join ();
        }
        return send_to_board (IronBCI::stop_command.c_str ());
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int IronBCI::release_session ()
{
    if (initialized)
    {
        if (keep_alive)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
    }
    if (serial)
    {
        serial->close_serial_port ();
        delete serial;
        serial = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void IronBCI::read_thread ()
{
    // format for data package, 27 bytes total
    /*
        Byte 0: 0xA0
        Byte 1: Sample Number
        Bytes 2-4: Data value for EEG channel 1
        Bytes 5-7: Data value for EEG channel 2
        Bytes 8-10: Data value for EEG channel 3
        Bytes 11-13: Data value for EEG channel 4
        Bytes 14-16: Data value for EEG channel 5
        Bytes 17-19: Data value for EEG channel 6
        Bytes 20-22: Data value for EEG channel 6
        Bytes 23-25: Data value for EEG channel 8
        Byte 26: 0xC0
    */
    int res;
    unsigned char b[26];
    float eeg_scale = 4.5 / float ((pow (2, 23) - 1)) / IronBCI::ads_gain * 1000000.;
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    while (keep_alive)
    {
        // check start byte
        res = serial->read_from_serial_port (b, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "Unable to read 1 byte");
            continue;
        }
        if (b[0] != IronBCI::start_byte)
        {
            continue;
        }

        // read remaining 26 bytes
        int remaining_bytes = 26;
        int pos = 0;
        while ((remaining_bytes > 0) && (keep_alive))
        {
            res = serial->read_from_serial_port (b + pos, remaining_bytes);
            remaining_bytes -= res;
            pos += res;
        }
        if (!keep_alive)
        {
            break;
        }
        // check stop byte
        if (b[25] != IronBCI::stop_byte)
        {
            safe_logger (spdlog::level::warn, "Wrong end byte {}", b[25]);
            continue;
        }

        // package num
        package[board_descr["package_num_channel"].get<int> ()] = (double)b[0];
        // eeg
        for (unsigned int i = 0; i < eeg_channels.size (); i++)
        {
            package[eeg_channels[i]] = eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
        }

        package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
    }
    delete[] package;
}
