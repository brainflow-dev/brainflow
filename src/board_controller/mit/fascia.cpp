#include <stdint.h>
#include <string.h>
#include <vector>

#include "custom_cast.h"
#include "fascia.h"
#include "timestamp.h"

#ifndef _WIN32
#include <errno.h>
#endif

constexpr int Fascia::transaction_size;
constexpr int Fascia::num_packages;
constexpr int Fascia::package_size;

Fascia::Fascia (struct BrainFlowInputParams params) : Board ((int)BoardIds::FASCIA_BOARD, params)
{
    socket = NULL;
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
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
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.ip_port == 0)
    {
        safe_logger (spdlog::level::err, "you need to provide port for local UDP server");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    socket = new SocketServerUDP (params.ip_port);
    int res = socket->bind ();
    if (res != (int)SocketServerUDPReturnCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::err, "failed to init socket: {}", res);
        delete socket;
        socket = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Fascia::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "config_board is not supported for Fascia.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Fascia::start_stream (int buffer_size, char *streamer_params)
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

    // no command to start streaming, its on all the time, just create thread to read it
    // if you add command, send it here
    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for data to ensure that everything is okay(its optional)
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, 5 * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        is_streaming = true;
        return state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in 3sec, stopping thread");
        is_streaming = true;
        stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
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
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        // no command to stop, if you add to firmware send it here
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
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
        free_packages ();
        initialized = false;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void Fascia::read_thread ()
{
    int res;
    unsigned char b[Fascia::transaction_size];
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    std::vector<int> eeg_channels = board_descr["eeg_channels"];

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
            if (state != (int)BrainFlowExitCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::info,
                    "received first package with {} bytes streaming is started", res);
                {
                    std::lock_guard<std::mutex> lk (m);
                    state = (int)BrainFlowExitCodes::STATUS_OK;
                }
                cv.notify_one ();
                safe_logger (spdlog::level::debug, "start streaming");
            }
        }

        // start parsing
        for (int cur_package = 0; cur_package < Fascia::num_packages; cur_package++)
        {
            int offset = cur_package * Fascia::package_size;
            int32_t package_num = 0;
            memcpy (&package_num, b + offset, 4);
            package[board_descr["package_num_channel"].get<int> ()] = (double)package_num;
            int32_t valid = 0;
            memcpy (&valid, b + 4 + offset, 4);
            package[board_descr["other_channels"][0].get<int> ()] = (double)valid;
            for (int i = 2, counter = 0; i < 10; i++, counter++)
            {
                float val;
                // sends data in volts
                memcpy (&val, b + offset + 8 + (i - 2) * 4, 4);
                package[board_descr["eeg_channels"][counter].get<int> ()] = 1000000.0 * val;
            }
            for (int i = 10, counter = 0; i < 13; i++, counter++)
            {
                package[board_descr["accel_channels"][counter].get<int> ()] =
                    accel_scale * cast_16bit_to_int32 (b + offset + 40 + (i - 10) * 2);
            }
            for (int i = 13, counter = 0; i < 16; i++, counter++)
            {
                package[board_descr["gyro_channels"][counter].get<int> ()] =
                    accel_scale * cast_16bit_to_int32 (b + offset + 40 + (i - 10) * 2);
            }

            int32_t eda, temperature, timestamp, ppg;
            memcpy (&eda, b + offset + 52, 4);
            memcpy (&temperature, b + offset + 56, 4);
            memcpy (&ppg, b + offset + 60, 4);
            memcpy (&timestamp, b + offset + 64, 4);
            package[board_descr["eda_channels"][0].get<int> ()] = (double)eda;
            package[board_descr["temperature_channels"][0].get<int> ()] = (double)temperature;
            package[board_descr["ppg_channels"][0].get<int> ()] = (double)ppg;
            package[board_descr["timestamp_channel"].get<int> ()] = get_timestamp ();

            push_package (package);
        }
    }
    delete[] package;
}
