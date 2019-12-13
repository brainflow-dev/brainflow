#include <chrono>
#include <stdint.h>
#include <string.h>

#include "custom_cast.h"
#include "novaxr.h"
#include "openbci_helpers.h"

#ifndef _WIN32
#include <errno.h>
#endif

constexpr int NovaXR::transaction_size;
constexpr int NovaXR::num_packages;
constexpr int NovaXR::package_size;
constexpr int NovaXR::num_channels;

NovaXR::NovaXR (struct BrainFlowInputParams params) : Board ((int)NOVAXR_BOARD, params)
{
    this->socket = NULL;
    this->is_streaming = false;
    this->keep_alive = false;
    this->initialized = false;
    this->state = SYNC_TIMEOUT_ERROR;
}

NovaXR::~NovaXR ()
{
    skip_logs = true;
    release_session ();
}

int NovaXR::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }
    if (params.ip_address.empty ())
    {
        safe_logger (spdlog::level::err, "ip address is empty");
        return INVALID_ARGUMENTS_ERROR;
    }
    if (params.ip_protocol == (int)IpProtocolType::TCP)
    {
        safe_logger (spdlog::level::err, "ip protocol is UDP for novaxr");
        return INVALID_ARGUMENTS_ERROR;
    }
    socket = new SocketClient (params.ip_address.c_str (), 2390, (int)SocketType::UDP);
    int res = socket->connect (NovaXR::transaction_size);
    if (res != (int)SocketReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        return GENERAL_ERROR;
    }
    initialized = true;
    return STATUS_OK;
}

int NovaXR::config_board (char *config)
{
    safe_logger (spdlog::level::debug, "Trying to config NovaXR with {}", config);
    int res = validate_config (config);
    if (res != STATUS_OK)
    {
        return res;
    }
    int len = strlen (config);
    res = socket->send (config, len);
    if (len != res)
    {
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        }
        safe_logger (spdlog::level::err, "Failed to config a board");
        return BOARD_WRITE_ERROR;
    }
    return STATUS_OK;
}

int NovaXR::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
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
    if (res != STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (NovaXR::num_channels, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    // start streaming
    res = socket->send ("b", 1);
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
        return BOARD_WRITE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 5 * sec, [this] { return this->state != SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        Board::board_logger->error ("no data received in 5sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        // more likely error occured due to wrong ip address, return UNABLE_TO_OPEN_PORT instead
        // SYNC_TIMEOUT_ERROR
        return UNABLE_TO_OPEN_PORT_ERROR;
    }
}

int NovaXR::stop_stream ()
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
        this->state = SYNC_TIMEOUT_ERROR;
        int res = socket->send ("s", 1);
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
            return BOARD_WRITE_ERROR;
        }
        return STATUS_OK;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int NovaXR::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        initialized = false;
        if (socket)
        {
            socket->close ();
            delete socket;
            socket = NULL;
        }
    }
    return STATUS_OK;
}

void NovaXR::read_thread ()
{
    /* ------ NovaXR packet format --------
     * Packet Byte [0]:     Packet Number
     * Packet Byte [1]:     PPG
     * Packet Byte [2:3]:   EDA
     * Packet Byte [4:6]:   EEG_FC 0
     * Packet Byte [7:9]:   EEG_FC 1
     * Packet Byte [10:12]: EEG_OL 0
     * Packet Byte [13:15]: EEG_OL 1
     * Packet Byte [16:18]: EEG_OL 2
     * Packet Byte [19:21]: EEG_OL 3
     * Packet Byte [22:24]: EEG_OL 4
     * Packet Byte [25:27]: EEG_OL 5
     * Packet Byte [28:30]: EEG_OL 6
     * Packet Byte [31:33]: EEG_OL 7
     * Packet Byte [34:36]: EOG 0
     * Packet Byte [37:39]: EOG 1
     * Packet Byte [40:42]: EMG 0
     * Packet Byte [43:45]: EMG 1
     * Packet Byte [46:48]: EMG 2
     * Packet Byte [49:51]: EMG 3
     * Packet Byte [52]:    Battery Level range: 0-100
     * Packet Byte [53:54]: Skin Temperature
     * Packet Byte [55]:    Firmware Error Code
     * Packet Byte [64:71]: Timestamp
     */

    int res;
    unsigned char b[NovaXR::transaction_size];
    while (keep_alive)
    {
        res = socket->recv (b, NovaXR::transaction_size);
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        }
        if (res != NovaXR::transaction_size)
        {
            safe_logger (spdlog::level::trace, "unable to read {} bytes, read {}",
                NovaXR::transaction_size, res);
            continue;
        }
        else
        {
            // inform main thread that everything is ok and first package was received
            if (this->state != STATUS_OK)
            {
                safe_logger (spdlog::level::info,
                    "received first package with {} bytes streaming is started", res);
                {
                    std::lock_guard<std::mutex> lk (this->m);
                    this->state = STATUS_OK;
                }
                this->cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }
        }

        for (int cur_package = 0; cur_package < NovaXR::num_packages; cur_package++)
        {
            double package[NovaXR::num_channels] = {0.};
            int offset = cur_package * NovaXR::package_size;
            // package num
            package[0] = (double)b[0 + offset];
            // eeg and emg
            for (int i = 4; i < 20; i++)
            {
                // put them directly after package num in brainflow
                package[i - 3] =
                    eeg_scale * (double)cast_24bit_to_int32 (b + offset + 4 + 3 * (i - 4));
            }
            int16_t temperature;
            memcpy (&temperature, b + 53 + offset, 2);
            package[17] = (double)b[1 + offset];                // ppg
            package[18] = cast_16bit_to_int32 (b + 2 + offset); // eda
            package[19] = temperature / 100.0;                  // temperature
            package[20] = (double)b[52 + offset];               // battery level

            double timestamp;
            memcpy (&timestamp, b + 64 + offset, 8);
            streamer->stream_data (package, NovaXR::num_channels, timestamp);
            db->add_data (timestamp, package);
        }
    }
}
