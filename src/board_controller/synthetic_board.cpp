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
    std::vector<int> exg_channels =
        board_descr["default"]["eeg_channels"]; // same channels for eeg\emg\ecg
    double *sin_phase_rad = new double[exg_channels.size ()];
    for (unsigned int i = 0; i < board_descr["default"]["eeg_channels"].size (); i++)
    {
        sin_phase_rad[i] = 0.0;
    }
    int sampling_rate = board_descr["default"]["sampling_rate"];
    int sleep_time = (int)((1000.0 / sampling_rate));
    std::uniform_real_distribution<double> dist_around_one (0.90, 1.10);
    uint64_t seed = std::chrono::high_resolution_clock::now ().time_since_epoch ().count ();
    std::mt19937 mt (static_cast<uint32_t> (seed));
    double accumulated_time_delta = 0.0;

    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    int num_aux_rows = board_descr["auxiliary"]["num_rows"];
    double *aux_package = new double[num_aux_rows];
    for (int i = 0; i < num_aux_rows; i++)
    {
        aux_package[i] = 0.0;
    }

    while (keep_alive)
    {
        auto start = std::chrono::high_resolution_clock::now ();
        package[board_descr["default"]["package_num_channel"].get<int> ()] = (double)counter;
        for (unsigned int i = 0; i < exg_channels.size (); i++)
        {
            double amplitude = 10.0 * (i + 1);
            double noise = 0.1 * (i + 1);
            double freq = 5.0 * (i + 1);
            int peak_frequency = (int)(sampling_rate / (i + 1));
            double shift = 0.05 * i;
            double range = (amplitude * noise) / 2.0;
            std::uniform_real_distribution<double> dist (0 - range, range);
            sin_phase_rad[i] += 2.0f * M_PI * freq / (double)sampling_rate;
            if (sin_phase_rad[i] > 2.0f * M_PI)
            {
                sin_phase_rad[i] -= 2.0f * M_PI;
            }
            if ((i > 5) &&
                ((counter % peak_frequency == 0) || ((counter - 1) % peak_frequency == 0) ||
                    (((counter + 1) % peak_frequency == 0))))
            {
                amplitude *= dist_around_one (mt) * 2;
            }
            package[exg_channels[i]] =
                amplitude + (amplitude + dist (mt)) * sqrt (2.0) * sin (sin_phase_rad[i] + shift);
        }
        for (int channel : board_descr["default"]["accel_channels"])
        {
            package[channel] = dist_around_one (mt) - 0.1;
        }
        for (int channel : board_descr["default"]["gyro_channels"])
        {
            package[channel] = dist_around_one (mt) - 0.1;
        }
        for (int channel : board_descr["default"]["eda_channels"])
        {
            package[channel] = dist_around_one (mt);
        }
        for (int chan_num = 0; chan_num < (int)board_descr["default"]["ppg_channels"].size ();
             chan_num++)
        {
            int channel = board_descr["default"]["ppg_channels"][chan_num];
            if (chan_num == 0)
            {
                package[channel] = 500.0 * dist_around_one (mt);
            }
            else
            {
                package[channel] = 253500.0 * dist_around_one (mt);
            }
        }
        for (int channel : board_descr["default"]["temperature_channels"])
        {
            package[channel] = dist_around_one (mt) / 10.0 + 36.5;
        }
        for (int channel : board_descr["default"]["resistance_channels"])
        {
            package[channel] = 1000.0 * dist_around_one (mt);
        }
        package[board_descr["default"]["battery_channel"].get<int> ()] =
            (dist_around_one (mt) - 0.1) * 100;
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (package); // use this method to submit data to buffers

        // push aux package
        for (int channel : board_descr["auxiliary"]["other_channels"])
        {
            aux_package[channel] = (double)channel;
        }
        aux_package[board_descr["auxiliary"]["timestamp_channel"].get<int> ()] = get_timestamp ();
        aux_package[board_descr["auxiliary"]["package_num_channel"].get<int> ()] = (double)counter;
        aux_package[board_descr["auxiliary"]["battery_channel"].get<int> ()] =
            (dist_around_one (mt) - 0.1) * 100;
        for (int channel : board_descr["auxiliary"]["accel_channels"])
        {
            aux_package[channel] = dist_around_one (mt) - 0.1;
        }
        for (int channel : board_descr["auxiliary"]["gyro_channels"])
        {
            aux_package[channel] = dist_around_one (mt) - 0.1;
        }
        for (int channel : board_descr["auxiliary"]["eda_channels"])
        {
            aux_package[channel] = dist_around_one (mt);
        }
        for (int chan_num = 0; chan_num < (int)board_descr["auxiliary"]["ppg_channels"].size ();
             chan_num++)
        {
            int channel = board_descr["auxiliary"]["ppg_channels"][chan_num];
            if (chan_num == 0)
            {
                aux_package[channel] = 500.0 * dist_around_one (mt);
            }
            else
            {
                aux_package[channel] = 253500.0 * dist_around_one (mt);
            }
        }
        for (int channel : board_descr["auxiliary"]["temperature_channels"])
        {
            aux_package[channel] = dist_around_one (mt) / 10.0 + 36.5;
        }
        for (int channel : board_descr["auxiliary"]["resistance_channels"])
        {
            aux_package[channel] = 1000.0 * dist_around_one (mt);
        }

        push_package (aux_package, (int)BrainFlowPresets::AUXILIARY_PRESET);

        if (sleep_time - accumulated_time_delta > 1)
        {
#ifdef _WIN32
            Sleep ((int)(sleep_time - accumulated_time_delta));
#else
            usleep ((int)(1000 * (sleep_time - accumulated_time_delta)));
#endif
        }

        counter++;
        auto stop = std::chrono::high_resolution_clock::now ();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds> (stop - start).count ();
        accumulated_time_delta += (duration / 1000.0 - sleep_time);
    }
    delete[] sin_phase_rad;
    delete[] package;
    delete[] aux_package;
}

int SyntheticBoard::config_board (std::string config, std::string &response)
{
    response = "Config:" + config;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

// if you use this board as a reference, more likely you dont need to implement this method
int SyntheticBoard::config_board_with_bytes (const char *bytes, int len)
{
    safe_logger (spdlog::level::info, "config_board_with_bytes for len: {}", len);
    for (int i = 0; i < len; i++)
    {
        safe_logger (spdlog::level::trace, "byte: {} value: {}", i, (int)bytes[i]);
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
