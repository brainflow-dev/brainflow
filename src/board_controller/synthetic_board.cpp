#include <chrono>
#include <fstream>
#include <math.h>
#include <random>
#include <string.h>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "synthetic_board.h"
#include "timestamp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


SyntheticBoard::SyntheticBoard (struct BrainFlowInputParams params)
    : Board ((int)BoardIds::SYNTHETIC_BOARD, params)
{
    is_streaming = false;
    keep_alive = false;
    initialized = false;
}

SyntheticBoard::~SyntheticBoard ()
{
    skip_logs = true;
    release_session ();
}

int SyntheticBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int SyntheticBoard::start_stream (int buffer_size, const char *streamer_params)
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

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int SyntheticBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int SyntheticBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
        initialized = false;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void SyntheticBoard::read_thread ()
{
    unsigned char counter = 0;
    std::vector<int> exg_channels = board_descr["eeg_channels"]; // same channels for eeg\emg\ecg
    double *sin_phase_rad = new double[exg_channels.size ()];
    for (unsigned int i = 0; i < board_descr["eeg_channels"].size (); i++)
    {
        sin_phase_rad[i] = 0.0;
    }
    int sampling_rate = board_descr["sampling_rate"];
    int batch_size = 15;
    int sleep_time = (int)((1000.0 / sampling_rate) * batch_size);
    std::uniform_real_distribution<double> dist_around_one (0.90, 1.10);
    uint64_t seed = std::chrono::high_resolution_clock::now ().time_since_epoch ().count ();
    std::mt19937 mt (static_cast<uint32_t> (seed));
    double accumulated_time_delta = 0.0;

    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    double timestamp = 0;
    while (keep_alive)
    {
        auto start = std::chrono::high_resolution_clock::now ();
        for (int num_in_batch = 0; num_in_batch < batch_size; num_in_batch++)
        {
            if (num_in_batch == 0)
            {
                timestamp = get_timestamp ();
            }
            package[board_descr["package_num_channel"].get<int> ()] = (double)counter;
            for (unsigned int i = 0; i < exg_channels.size (); i++)
            {
                double amplitude = 10.0 * (i + 1);
                double noise = 0.1 * (i + 1);
                double freq = 5.0 * (i + 1);
                double shift = 0.05 * i;
                double range = (amplitude * noise) / 2.0;
                std::uniform_real_distribution<double> dist (0 - range, range);
                sin_phase_rad[i] += 2.0f * M_PI * freq / (double)sampling_rate;
                if (sin_phase_rad[i] > 2.0f * M_PI)
                {
                    sin_phase_rad[i] -= 2.0f * M_PI;
                }
                package[exg_channels[i]] =
                    (amplitude + dist (mt)) * sqrt (2.0) * sin (sin_phase_rad[i] + shift);
            }
            for (int channel : board_descr["accel_channels"])
            {
                package[channel] = dist_around_one (mt) - 0.1;
            }
            for (int channel : board_descr["gyro_channels"])
            {
                package[channel] = dist_around_one (mt) - 0.1;
            }
            for (int channel : board_descr["eda_channels"])
            {
                package[channel] = dist_around_one (mt);
            }
            for (int channel : board_descr["ppg_channels"])
            {
                package[channel] = 5000.0 * dist_around_one (mt);
            }
            for (int channel : board_descr["temperature_channels"])
            {
                package[channel] = dist_around_one (mt) / 10.0 + 36.5;
            }
            for (int channel : board_descr["resistance_channels"])
            {
                package[channel] = 1000.0 * dist_around_one (mt);
            }
            package[board_descr["battery_channel"].get<int> ()] =
                (dist_around_one (mt) - 0.1) * 100;
            package[board_descr["timestamp_channel"].get<int> ()] =
                timestamp + num_in_batch / (double)sampling_rate;

            push_package (package); // use this method to submit data to buffers

            counter++;
        }
        auto stop = std::chrono::high_resolution_clock::now ();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds> (stop - start).count ();
        if (duration < sleep_time)
        {
#ifdef _WIN32
            Sleep (sleep_time - duration);
#else
            usleep ((sleep_time - duration) * 1000);
#endif
        }
    }
    delete[] sin_phase_rad;
    delete[] package;
}

int SyntheticBoard::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
