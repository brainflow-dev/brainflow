#include <chrono>
#include <fstream>
#include <math.h>
#include <random>
#include <string.h>
#include <string>

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


constexpr int SyntheticBoard::package_size;


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
    safe_logger (spdlog::level::trace, "prepare session");
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int SyntheticBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::trace, "start stream");
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

    if (streamer)
    {
        delete streamer;
        streamer = NULL;
    }
    if (db)
    {
        delete db;
        db = NULL;
    }
    int res = prepare_streamer (streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (SyntheticBoard::package_size, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int SyntheticBoard::stop_stream ()
{
    safe_logger (spdlog::level::trace, "stop stream");
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
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int SyntheticBoard::release_session ()
{
    safe_logger (spdlog::level::trace, "release session");
    if (initialized)
    {
        stop_stream ();

        if (db)
        {
            delete db;
            db = NULL;
        }
        initialized = false;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void SyntheticBoard::read_thread ()
{
    unsigned char counter = 0;
    constexpr int exg_channels = 16;
    constexpr int imu_channels = 6; // accel + gyro
    double sin_phase_rad[exg_channels] = {0.0};
    double package[SyntheticBoard::package_size] = {0.0};
    int sampling_rate = 250;
    std::normal_distribution<double> accel_dist (0.0, 0.35);
    std::normal_distribution<double> temperature_dist (36.0, 0.5);
    std::normal_distribution<double> dist_mean_thousand (1000.0, 200.0);
    std::normal_distribution<double> eda_dist (1.08, 0.01);

    uint64_t seed = std::chrono::high_resolution_clock::now ().time_since_epoch ().count ();
    std::mt19937 mt (static_cast<uint32_t> (seed));

    while (keep_alive)
    {
        package[0] = (double)counter;
        // exg
        for (int i = 0; i < exg_channels; i++)
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
            package[i + 1] = (amplitude + dist (mt)) * sqrt (2.0) * sin (sin_phase_rad[i] + shift);
        }
        // accel and gyro
        for (int i = 0; i < imu_channels; i++)
        {
            package[i + 1 + exg_channels] = accel_dist (mt);
        }
        // eda
        package[23] = eda_dist (mt);
        // ppg
        package[24] = 5.0 * dist_mean_thousand (mt);
        package[25] = 5.0 * dist_mean_thousand (mt);
        // temperature
        package[26] = temperature_dist (mt);
        // resistance (add just 2 channels)
        package[27] = dist_mean_thousand (mt);
        package[28] = dist_mean_thousand (mt);
        // battery
        package[29] = 95.0;

        double timestamp = get_timestamp ();
        db->add_data (timestamp, package);
        streamer->stream_data (package, SyntheticBoard::package_size, timestamp);
        counter++;
#ifdef _WIN32
        // with 3 sampling rate is 250 on all machines
        Sleep (3);
#else
        usleep (3000);
#endif
    }
}

int SyntheticBoard::config_board (std::string config, std::string &response)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
