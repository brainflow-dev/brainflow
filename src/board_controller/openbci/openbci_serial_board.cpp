#include <string.h>

#include "openbci_serial_board.h"
#include "serial.h"


OpenBCISerialBoard::OpenBCISerialBoard (struct BrainFlowInputParams params, int board_id)
    : Board (board_id, params)
{
    serial = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

OpenBCISerialBoard::~OpenBCISerialBoard ()
{
    skip_logs = true;
    release_session ();
}

int OpenBCISerialBoard::open_port ()
{
    if (serial->is_port_open ())
    {
        safe_logger (spdlog::level::err, "port {} already open", serial->get_port_name ());
        return (int)BrainFlowExitCodes::PORT_ALREADY_OPEN_ERROR;
    }

    safe_logger (spdlog::level::info, "opening port {}", serial->get_port_name ());
    int res = serial->open_serial_port ();
    if (res < 0)
    {
        safe_logger (spdlog::level::err,
            "Make sure you provided correct port name and have permissions to open it(run with "
            "sudo/admin). Also, close all other apps using this port.");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    safe_logger (spdlog::level::trace, "port {} is open", serial->get_port_name ());
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int OpenBCISerialBoard::config_board (std::string config, std::string &response)
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
    safe_logger (spdlog::level::warn,
        "If you change gain you may need to rescale data, in data returned by BrainFlow we use "
        "gain 24 to convert int24 to uV");
    return res;
}

int OpenBCISerialBoard::send_to_board (const char *msg)
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

int OpenBCISerialBoard::send_to_board (const char *msg, std::string &response)
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

std::string OpenBCISerialBoard::read_serial_response ()
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

int OpenBCISerialBoard::set_port_settings ()
{
    int res = serial->set_serial_port_settings (1000, false);
    if (res < 0)
    {
        safe_logger (spdlog::level::err, "Unable to set port settings, res is {}", res);
        return (int)BrainFlowExitCodes::SET_PORT_ERROR;
    }
    safe_logger (spdlog::level::trace, "set port settings");
    return send_to_board ("v");
}

int OpenBCISerialBoard::status_check ()
{
    unsigned char buf[1];
    int count = 0;
    int max_empty_seq = 5;
    int num_empty_attempts = 0;

    for (int i = 0; i < 500; i++)
    {
        int res = serial->read_from_serial_port (buf, 1);
        if (res > 0)
        {
            num_empty_attempts = 0;
            // board is ready if there are '$$$'
            if (buf[0] == '$')
            {
                count++;
            }
            else
            {
                count = 0;
            }
            if (count == 3)
            {
                return (int)BrainFlowExitCodes::STATUS_OK;
            }
        }
        else
        {
            num_empty_attempts++;
            if (num_empty_attempts > max_empty_seq)
            {
                safe_logger (spdlog::level::err, "board doesnt send welcome characters!");
                return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
            }
        }
    }
    return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
}

int OpenBCISerialBoard::prepare_session ()
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

    int initted = status_check ();
    if (initted != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete serial;
        serial = NULL;
        return initted;
    }

    int send_res = send_to_board ("d");
    if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return send_res;
    }
    // cyton sends response back, clean serial buffer and analyze response
    std::string response = read_serial_response ();
    if (response.substr (0, 7).compare ("Failure") == 0)
    {
        safe_logger (spdlog::level::err,
            "Board config error, probably dongle is inserted but Cyton is off.");
        safe_logger (spdlog::level::trace, "read {}", response.c_str ());
        delete serial;
        serial = NULL;
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int OpenBCISerialBoard::start_stream (int buffer_size, const char *streamer_params)
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

    // start streaming
    int send_res = send_to_board ("b");
    if (send_res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return send_res;
    }
    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int OpenBCISerialBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        if (streaming_thread.joinable ())
        {
            streaming_thread.join ();
        }

        return send_to_board ("s");
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int OpenBCISerialBoard::release_session ()
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
