#include <chrono>
#include <stdint.h>
#include <string.h>

#include "custom_cast.h"
#include "fascia.h"
#include "openbci_helpers.h"
#include "timestamp.h"

#ifndef _WIN32
#include <errno.h>
#endif

constexpr int Fascia::transaction_size;
constexpr int Fascia::num_packages;
constexpr int Fascia::package_size;
constexpr int Fascia::num_channels;

Fascia::Fascia (struct BrainFlowInputParams params) : Board ((int)FASCIA_BOARD, params)
{
    this->socket = NULL;
    this->is_streaming = false;
    this->keep_alive = false;
    this->initialized = false;
    this->state = SYNC_TIMEOUT_ERROR;
}

Fascia::~Fascia ()
{
    skip_logs = true;
    release_session ();
}

int Fascia::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }
    if (params.ip_port == 0)
    {
        safe_logger (spdlog::level::err, "you need to provide port for local UDP server");
        return INVALID_ARGUMENTS_ERROR;
    }
    socket = new SocketServerUDP (params.ip_port);
    int res = socket->bind ();
    if (res != (int)SocketServerUDPCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        delete socket;
        socket = NULL;
        return GENERAL_ERROR;
    }
    initialized = true;
    return STATUS_OK;
}

int Fascia::config_board (char *config)
{
    safe_logger (spdlog::level::err, "config_board is not supported for Fascia.");
    return UNSUPPORTED_BOARD_ERROR;
}

int Fascia::start_stream (int buffer_size, char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return BOARD_NOT_CREATED_ERROR;
    }
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
    db = new DataBuffer (Fascia::num_channels, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    // no command to start streaming, its on all the time, just create thread to read it
    // if you add command, send it here
    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay(its optional)
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 5 * sec, [this] { return this->state != SYNC_TIMEOUT_ERROR; }))
    {
        this->is_streaming = true;
        return this->state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 3sec, stopping thread");
        this->is_streaming = true;
        this->stop_stream ();
        return SYNC_TIMEOUT_ERROR;
    }
}

int Fascia::stop_stream ()
{
    safe_logger (spdlog::level::trace, "stopping thread");
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
        // no command to stop, if you add to firmware send it here
        return STATUS_OK;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Fascia::release_session ()
{
    safe_logger (spdlog::level::trace, "releasing session");
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

void Fascia::read_thread ()
{
    int res;
    unsigned char b[Fascia::transaction_size];
    while (keep_alive)
    {
        res = socket->recv (b, Fascia::transaction_size);
        // log socket error
        if (res == -1)
        {
#ifdef _WIN32
            safe_logger (spdlog::level::err, "WSAGetLastError is {}", WSAGetLastError ());
#else
            safe_logger (spdlog::level::err, "errno {} message {}", errno, strerror (errno));
#endif
        }
        // log amount of bytes read
        if (res != Fascia::transaction_size)
        {
            safe_logger (spdlog::level::trace, "unable to read {} bytes, read {}",
                Fascia::transaction_size, res);
            continue;
        }
        else
        {
            // inform main thread that first package was received
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

        // start parsing
        for (int cur_package = 0; cur_package < Fascia::num_packages; cur_package++)
        {
            double package[Fascia::num_channels] = {0.};
            int offset = cur_package * Fascia::package_size;
            // package num
            int32_t package_num = 0;
            memcpy (&package_num, b + offset, 4);
            package[0] = (double)package_num;
            // valid indicator
            int32_t valid = 0;
            memcpy (&valid, b + 4 + offset, 4);
            package[1] = (double)valid;
            // 8 adc datapoints
            for (int i = 2; i < 10; i++)
            {
                float val;
                // sends data in volts
                memcpy (&val, b + offset + 8 + (i - 2) * 4, 4);
                package[i] = 1000000.0 * val; // convert to uV(its a standard unit in brainflow)
            }
            // IMU (need to cast from int16)
            for (int i = 10; i < 16; i++)
            {
                package[i] = accel_scale * cast_16bit_to_int32 (b + offset + 40 + (i - 10) * 2);
            }

            int32_t eda, temperature, timestamp, ppg;
            // I hope I didnt make a mistake in indexes
            memcpy (&eda, b + offset + 52, 4);
            memcpy (&temperature, b + offset + 56, 4);
            memcpy (&ppg, b + offset + 60, 4);
            memcpy (&timestamp, b + offset + 64, 4);
            package[16] = (double)eda;
            package[17] = (double)temperature;
            package[18] = (double)ppg;

            /*
              in BrainFlow timestamp is required for all boards and it is in UNIX format
              its not crucial but will be nice to convert to UNIX format for consistency with other
              devices check novaxr or cyton(there are 2 different approaches for timestamps)
              also in users code it will be possible to compare this timestamps with smth like
              import time
              time.time()
            */
            streamer->stream_data (package, Fascia::num_channels, (double)timestamp);
            db->add_data ((double)timestamp, package);
        }
    }
}
