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

#include "openbci_helpers.h"
#include "synthetic_board.h"
#include "timestamp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SyntheticBoard::SyntheticBoard (struct BrainFlowInputParams params)
    : Board ((int)SYNTHETIC_BOARD, params)
{
    this->is_streaming = false;
    this->keep_alive = false;
    this->initialized = false;
    this->num_channels = 8;
    this->amplitude = 1000;
    this->shift = 0.3f;
    this->noise = 0.75f;
    this->sampling_rate = 256;
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
        return STATUS_OK;
    }
    initialized = true;
    return STATUS_OK;
}

int SyntheticBoard::start_stream (int buffer_size)
{
    safe_logger (spdlog::level::trace, "start stream");
    if (this->is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "invalid array size");
        return INVALID_BUFFER_SIZE_ERROR;
    }

    if (this->db)
    {
        delete this->db;
        this->db = NULL;
    }
    // here num channels means num_eeg_channels,total number of channels is num_eeg_channels + 1 +3
    this->db = new DataBuffer (num_channels + 4, buffer_size);
    if (!this->db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer with size {}", buffer_size);
        return INVALID_BUFFER_SIZE_ERROR;
    }

    this->keep_alive = true;
    this->streaming_thread = std::thread ([this] { this->read_thread (); });
    this->is_streaming = true;
    return STATUS_OK;
}

int SyntheticBoard::stop_stream ()
{
    safe_logger (spdlog::level::trace, "stop stream");
    if (this->is_streaming)
    {
        this->keep_alive = false;
        this->is_streaming = false;
        this->streaming_thread.join ();
        return STATUS_OK;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int SyntheticBoard::release_session ()
{
    safe_logger (spdlog::level::trace, "release session");
    if (this->initialized)
    {
        this->stop_stream ();

        if (this->db)
        {
            delete this->db;
            this->db = NULL;
        }
        this->initialized = false;
    }
    return STATUS_OK;
}


void SyntheticBoard::read_thread ()
{
    // predefined based sin wave
    unsigned char counter = 0;
    constexpr int num_samples = 256;
    float base_wave[256];
    for (int i = 0; i < num_samples; i++)
    {
        float rads = (float)(M_PI / 180.0f);
        base_wave[i] = this->amplitude * sin (1.8f * i * rads + this->shift);
    }
    // eeg channels + 3 accel channels + package num
    double *package = new double[this->num_channels + 3 + 1];
    // random distr for noise
    uint64_t seed = std::chrono::high_resolution_clock::now ().time_since_epoch ().count ();
    std::mt19937 mt (static_cast<uint32_t> (seed));
    float max_noise = (this->noise > 0.001f) ? this->noise : 0.001f;
    float range = (this->amplitude * max_noise) / 2.0f;
    safe_logger (spdlog::level::info, "noise range is {}:{}", -range, range);
    safe_logger (spdlog::level::info, "amplitude is {}", this->amplitude);
    safe_logger (spdlog::level::info, "shift is {}", this->shift);
    std::uniform_real_distribution<float> dist (0 - range, range);

    while (this->keep_alive)
    {
        // package num
        package[0] = (double)counter;
        // eeg
        for (int i = 0; i < this->num_channels; i++)
        {
            package[i + 1] = base_wave[counter] + dist (mt);
        }
        // accel
        package[1 + this->num_channels] = dist (mt);
        package[2 + this->num_channels] = dist (mt);
        package[3 + this->num_channels] = dist (mt);

        db->add_data (get_timestamp (), package);
        counter++;
#ifdef _WIN32
        Sleep ((int)(1000 / this->sampling_rate));
#else
        usleep ((int)(1000000 / this->sampling_rate));
#endif
    }
    delete[] package;
}

int SyntheticBoard::config_board (char *config)
{
    return validate_config (config);
}
