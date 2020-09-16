#include <chrono>
#include <string.h>

#include "custom_cast.h"
#include "ironbci.h"
#include "timestamp.h"

#ifndef _WIN32
#include <errno.h>
#endif

constexpr int IronBCI::transaction_size;
constexpr int IronBCI::num_packages;
constexpr int IronBCI::tcp_port;
constexpr int IronBCI::udp_port;
constexpr int IronBCI::package_size;
constexpr int IronBCI::num_channels;
constexpr int IronBCI::ads_gain;
const std::string IronBCI::start_command = "b";
const std::string IronBCI::stop_command = "s";


IronBCI::IronBCI (struct BrainFlowInputParams params) : Board ((int)BoardIds::IRONBCI_BOARD, params)
{
    this->command_socket = NULL;
    this->data_socket = NULL;
    this->keep_alive = false;
    this->is_streaming = false;
    this->initialized = false;
    this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

IronBCI::~IronBCI ()
{
    skip_logs = true;
    release_session ();
}

int IronBCI::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.ip_address.empty ())
    {
        safe_logger (spdlog::level::info, "use default IP address 192.168.4.1");
        params.ip_address = "192.168.4.1";
    }
    command_socket = new SocketClientTCP (params.ip_address.c_str (), IronBCI::tcp_port);
    int res = command_socket->connect ();
    if (res != (int)SocketClientTCPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init command_socket: {}", res);
        delete command_socket;
        command_socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    data_socket = new SocketClientUDP (params.ip_address.c_str (), IronBCI::udp_port);
    res = data_socket->connect ();
    if (res != (int)SocketClientUDPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init data_socket: {}", res);
        delete data_socket;
        data_socket = NULL;
        delete command_socket;
        command_socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int IronBCI::config_board (char *config)
{
    safe_logger (spdlog::level::err, "config_board is not supported for IronBCI");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int IronBCI::start_stream (int buffer_size, char *streamer_params)
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
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    if (db)
    {
        delete db;
        db = NULL;
    }
    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }

    int res = prepare_streamer (streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (IronBCI::num_channels, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        delete db;
        db = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    // start streaming
    res = command_socket->send (IronBCI::start_command.c_str (), 1);
    if (res != 1)
    {
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        }
        safe_logger (spdlog::level::err, "Failed to send a command to board");
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 5 * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 3sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
}

int IronBCI::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        if (streamer)
        {
            delete streamer;
            streamer = NULL;
        }
        this->state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        int res = command_socket->send (IronBCI::stop_command.c_str (), 1);
        if (res != 1)
        {
            if (res == -1)
            {
#ifdef _WIN32
                safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
                safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
            }
            safe_logger (spdlog::level::err, "Failed to send a command to board");
            return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
        }
        return (int)BrainFlowExitCodes::STATUS_OK;
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
        if (is_streaming)
        {
            stop_stream ();
        }
        initialized = false;
        if (command_socket)
        {
            command_socket->close ();
            delete command_socket;
            command_socket = NULL;
        }
        if (data_socket)
        {
            data_socket->close ();
            delete data_socket;
            data_socket = NULL;
        }
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void IronBCI::read_thread ()
{
    int res;
    unsigned char b[IronBCI::transaction_size];
    float eeg_scale = 4.5 / float((pow (2, 23) - 1)) / IronBCI::ads_gain * 1000000.;

    while (keep_alive)
    {
        res = data_socket->recv (b, IronBCI::transaction_size);

        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        }
        if (res != IronBCI::transaction_size)
        {
            safe_logger (spdlog::level::trace, "unable to read {} bytes, read {}",
                IronBCI::transaction_size, res);
            continue;
        }
        else
        {
            // inform main thread that everything is ok and first package was received
            if (this->state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::info,
                    "received first package with {} bytes streaming is started", res);
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }
        }

        for (int cur_package = 0; cur_package < IronBCI::num_packages; cur_package++)
        {
            // format for data package, 25 bytes total
            /*
                Byte 0: Sample Number
                Bytes 1-3: Data value for EEG channel 1
                Bytes 4-6: Data value for EEG channel 2
                Bytes 7-9: Data value for EEG channel 3
                Bytes 10-12: Data value for EEG channel 4
                Bytes 13-15: Data value for EEG channel 5
                Bytes 16-18: Data value for EEG channel 6
                Bytes 19-21: Data value for EEG channel 6
                Bytes 22-24: Data value for EEG channel 8
            */
            double package[IronBCI::num_channels] = {0.};
            int offset = cur_package * IronBCI::package_size;
            // package num
            package[0] = (double)b[0 + offset];
            // eeg
            for (int i = 0; i < 8; i++)
            {
                package[i + 1] = eeg_scale * cast_24bit_to_int32 (b + offset + 1 + 3 * i);
            }
            double timestamp = get_timestamp ();
            streamer->stream_data (package, IronBCI::num_channels, timestamp);
            db->add_data (timestamp, package);
        }
    }
}
