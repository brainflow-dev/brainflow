#include <string.h>

#include "enophone.h"

#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"


Enophone::Enophone (struct BrainFlowInputParams params)
    : BTLibBoard ((int)BoardIds::ENOPHONE_BOARD, params)
{
    keep_alive = false;
    state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
}

Enophone::~Enophone ()
{
    skip_logs = true;
    release_session ();
}

int Enophone::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        safe_logger (spdlog::level::err, "You need to call prepare_session before config_board");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
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

    res = bluetooth_open_device ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    // wait for the 1st package received
    std::unique_lock<std::mutex> lk (this->m);
    auto sec = std::chrono::seconds (1);
    int num_secs = 5;
    if (cv.wait_for (lk, num_secs * sec,
            [this] { return this->state != (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR; }))
    {
        return state;
    }
    else
    {
        safe_logger (spdlog::level::err, "no data received in {} sec, stopping thread", num_secs);
        stop_stream ();
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
}

int Enophone::stop_stream ()
{
    if (keep_alive)
    {
        keep_alive = false;
        streaming_thread.join ();
        state = (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
        return bluetooth_close_device ();
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Enophone::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
    }
    return BTLibBoard::release_session ();
}

void Enophone::read_thread ()
{
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    constexpr int buf_size = 20;
    char temp_buffer[buf_size];
    for (int i = 0; i < buf_size; i++)
    {
        temp_buffer[i] = 0;
    }

    while (keep_alive)
    {
        bool is_ready = false;
        // check first byte is 'b'
        int res = bluetooth_get_data (temp_buffer, 1);
        if ((res != 1) || (temp_buffer[0] != 'b'))
        {
            continue;
        }
        double timestamp = get_timestamp ();
        // notify main thread that 1st byte received
        if (state != (int)BrainFlowExitCodes::STATUS_OK)
        {
            {
                std::lock_guard<std::mutex> lk (m);
                state = (int)BrainFlowExitCodes::STATUS_OK;
            }
            cv.notify_one ();
            safe_logger (spdlog::level::debug, "start streaming");
        }

        // check second byte is 'S'
        while ((keep_alive) && (res >= 0))
        {
            res = bluetooth_get_data (temp_buffer + 1, 1);
            if (res == 1)
            {
                if (temp_buffer[1] == 'S')
                {
                    is_ready = true;
                }
                break;
            }
        }

        // first two bytes received, ready to read data bytes
        if (is_ready)
        {
            while ((keep_alive) && (res >= 0))
            {
                res = bluetooth_get_data (temp_buffer + 2, buf_size - 2);
                if (res == buf_size - 2)
                {
                    std::vector<int> eeg_channels = board_descr["eeg_channels"];
                    for (int i = 0; i < 5; i++)
                    {
                        int32_t val = 0;
                        memcpy (&val, temp_buffer + i * sizeof (int32_t), sizeof (int32_t));
                        val = swap_endians (val);
                        if (i == 0)
                        {
                            package[board_descr["package_num_channel"].get<int> ()] = val % 65536;
                        }
                        else
                        {
                            double value = ((double)(val / 256)) * 5.0 / 8388608.0;
                            double gain = 100.0;
                            value = value / gain * 1000000.0;
                            package[eeg_channels[i - 1]] = value;
                        }
                    }
                    package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
                    push_package (package);
                    break;
                }
            }
        }
    }
    delete[] package;
}

int Enophone::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::debug, "config_board is not supported for Enophone");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

std::string Enophone::get_name_selector ()
{
    return "nophone";
}
