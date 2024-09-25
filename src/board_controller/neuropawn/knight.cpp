#include <math.h>
#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "knight.h"
#include "serial.h"
#include "timestamp.h"

constexpr int Knight::start_byte;
constexpr int Knight::end_byte;

Knight::Knight (int board_id, struct BrainFlowInputParams params) : Board (board_id, params)
{
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

Knight::~Knight ()
{
    skip_logs = true;
    release_session ();
}

int Knight::prepare_session ()
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

int Knight::start_stream (int buffer_size, const char *streamer_params)
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

int Knight::stop_stream ()
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

int Knight::release_session ()
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

void Knight::read_thread ()
{
    /*
    [0] 1 Byte: Start byte
    [1] 2 Byte: Sample Number
    [2-3] 3-4 Bytes: EXG channel 1
    [4-5] 5-6 Bytes: Data value for EXG channel 2
    [6-7] 7-8 Bytes: Data value for EXG channel 3
    [8-9] 9-10 Bytes: Data value for EXG channel 4
    [10-11] 11-12 Bytes: Data value for EXG channel 5
    [12-13] 13-14 Bytes: Data value for EXG channel 6
    [14-15] 15-16 Bytes: Data value for EXG channel 7
    [16-17] 17-18 Bytes: Data value for EXG channel 8
    [18] 19 Byte: Data LOFF STATP
    [19] 20 Byte: Data LOFF STATN
    [20] 21 Byte: End byte
    */

    int res;
    unsigned char b[20] = {0};
    float eeg_scale = 4 / float ((pow (2, 23) - 1)) / 12 * 1000000.;
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    bool first_package_received = false;

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    std::vector<int> other_channels = board_descr["default"]["other_channels"];

    while (keep_alive)
    {
        // checking the start byte
        res = serial->read_from_serial_port (b, 1);
        if (res != 1)
        {
            safe_logger (spdlog::level::debug, "unable to read 1 byte, {}");
            continue;
        }
        if (b[0] != Knight::start_byte)
        {
            continue;
        }

        int remaining_bytes = 20;
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

        if (b[19] != Knight::end_byte)
        {
            safe_logger (spdlog::level::warn, "Wrong end byte {}", b[19]);
            continue;
        }

        // package number CHANGE TO 1 if not working
        package[board_descr["default"]["package_num_channel"].get<int> ()] = (double)b[0];

        // exg data retrieval
        for (unsigned int i = 0; i < eeg_channels.size (); i++)
        {
            package[eeg_channels[i]] =
                eeg_scale * cast_16bit_to_int32 (b + 1 + 2 * i); // CHANGE TO 2+2*i if not working
        }

        // other channel data retrieval
        package[other_channels[0]] = (double)b[17]; // LOFF STATP
        package[other_channels[1]] = (double)b[18]; // LOFF STATN

        // time stamp channel
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (package);
    }
    delete[] package;
}

int Knight::open_port ()
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

int Knight::set_port_settings ()
{
    int res = serial->set_serial_port_settings (1000, false);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set port settings, res is {}", res);
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    res = serial->set_custom_baudrate (115200);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set custom baud rate, res is {}", res);
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    safe_logger (spdlog::level::trace, "set port settings");
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Knight::config_board (std::string config, std::string &response)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (is_streaming)
    {
        safe_logger (spdlog::level::warn,
            "You are changing board params during streaming, it may lead to sync mismatch between "
            "data acquisition thread and device");
        res = send_to_board (config.c_str ());
    }
    else
    {
        // read response if streaming is not running
        res = send_to_board (config.c_str (), response);
    }

    return res;
}

int Knight::send_to_board (const char *msg)
{
    int length = (int)strlen (msg);
    safe_logger (spdlog::level::debug, "sending {} to the board", msg);
    int res = serial->send_to_serial_port ((const void *)msg, length);
    if (res != length)
    {
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Knight::send_to_board (const char *msg, std::string &response)
{
    int length = (int)strlen (msg);
    safe_logger (spdlog::level::debug, "sending {} to the board", msg);
    int res = serial->send_to_serial_port ((const void *)msg, length);
    if (res != length)
    {
        response = "";
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    response = read_serial_response ();

    return (int)BrainFlowExitCodes::STATUS_OK;
}

std::string Knight::read_serial_response ()
{
    constexpr int max_tmp_size = 4096;
    unsigned char tmp_array[max_tmp_size];
    unsigned char tmp;
    int tmp_id = 0;
    while (serial->read_from_serial_port (&tmp, 1) == 1)
    {
        if (tmp_id < max_tmp_size)
        {
            tmp_array[tmp_id] = tmp;
            tmp_id++;
        }
        else
        {
            serial->flush_buffer ();
            break;
        }
    }
    tmp_id = (tmp_id == max_tmp_size) ? tmp_id - 1 : tmp_id;
    tmp_array[tmp_id] = '\0';

    return std::string ((const char *)tmp_array);
}