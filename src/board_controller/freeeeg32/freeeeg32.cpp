#include <math.h>
#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "freeeeg32.h"
#include "serial.h"
#include "timestamp.h"


constexpr int FreeEEG32::start_byte;
constexpr int FreeEEG32::end_byte;
constexpr double FreeEEG32::ads_gain;
constexpr double FreeEEG32::ads_vref;


FreeEEG32::FreeEEG32 (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::FREEEEG32_BOARD, params)
{
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

FreeEEG32::~FreeEEG32 ()
{
    skip_logs = true;
    release_session ();
}

int FreeEEG32::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.serial_port.empty ())
    {
        safe_logger (spdlog::level::err, "serial port is empty");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    serial = Serial::create (params.serial_port.c_str (), this);
    int port_open = open_port ();
    if (port_open != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete serial;
        serial = NULL;
        return port_open;
    }

    int set_settings = set_port_settings ();
    if (set_settings != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete serial;
        serial = NULL;
        return set_settings;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int FreeEEG32::start_stream (int buffer_size, const char *streamer_params)
{
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

    serial->flush_buffer ();

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int FreeEEG32::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        if (streaming_thread.joinable ())
        {
            streaming_thread.join ();
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int FreeEEG32::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
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

void FreeEEG32::read_thread ()
{
    int res;
    constexpr int max_size = 200; // random value bigger than package size which is unknown
    unsigned char b[max_size] = {0};
    // dont know exact package size and it can be changed with new firmware versions, its >=
    // min_package_size and we can check start\stop bytes
    constexpr int min_package_size = 1 + 32 * 3;
    float eeg_scale =
        FreeEEG32::ads_vref / float ((pow (2, 23) - 1)) / FreeEEG32::ads_gain * 1000000.;
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    bool first_package_received = false;

    std::vector<int> eeg_channels = board_descr["eeg_channels"];

    while (keep_alive)
    {
        int pos = 0;
        bool complete_package = false;
        while ((keep_alive) && (pos < max_size - 2))
        {
            res = serial->read_from_serial_port (b + pos, 1);
            int prev_id = (pos <= 0) ? 0 : pos - 1;
            if ((b[pos] == FreeEEG32::start_byte) && (b[prev_id] == FreeEEG32::end_byte) &&
                (pos >= min_package_size))
            {
                complete_package = true;
                break;
            }
            pos += res;
        }
        if (complete_package)
        {
            // handle the case that we start reading in the middle of data stream
            if (!first_package_received)
            {
                first_package_received = true;
                continue;
            }
            package[board_descr["package_num_channel"].get<int> ()] = (double)b[0];
            for (unsigned int i = 0; i < eeg_channels.size (); i++)
            {
                package[eeg_channels[i]] = (double)eeg_scale * cast_24bit_to_int32 (b + 1 + 3 * i);
            }
            package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();
            push_package (package);
        }
        else
        {
            safe_logger (
                spdlog::level::trace, "stopped with pos: {}, keep_alive: {}", pos, keep_alive);
        }
    }
    delete[] package;
}

int FreeEEG32::open_port ()
{
    if (serial->is_port_open ())
    {
        safe_logger (spdlog::level::err, "port {} already open", serial->get_port_name ());
        return (int)BrainFlowExitCodes::PORT_ALREADY_OPEN_ERROR;
    }

    safe_logger (spdlog::level::info, "openning port {}", serial->get_port_name ());
    int res = serial->open_serial_port ();
    if (res < 0)
    {
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    safe_logger (spdlog::level::trace, "port {} is open", serial->get_port_name ());
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int FreeEEG32::set_port_settings ()
{
    int res = serial->set_serial_port_settings (1000, false);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set port settings, res is {}", res);
#ifndef _WIN32
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
#endif
    }
    res = serial->set_custom_baudrate (921600);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set custom baud rate, res is {}", res);
#ifndef _WIN32
        // Setting the baudrate may return an error on Windows for some serial drivers.
        // We do not throw an exception, because it will still work with USB.
        // Optical connection will fail, though.
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
#endif
    }
    safe_logger (spdlog::level::trace, "set port settings");
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int FreeEEG32::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "FreeEEG32 doesn't support board configuration.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
