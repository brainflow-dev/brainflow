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


constexpr int SyntheticBoard::package_size;


SyntheticBoard::SyntheticBoard (struct BrainFlowInputParams params)
    : Board ((int)SYNTHETIC_BOARD, params)
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
        return STATUS_OK;
    }

    initialized = true;
    return STATUS_OK;
}

int SyntheticBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::trace, "start stream");
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
    if (res != STATUS_OK)
    {
        return res;
    }
    db = new DataBuffer (SyntheticBoard::package_size, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return STATUS_OK;
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
    return STATUS_OK;
}

void SyntheticBoard::read_thread ()
{
    // predefined sin wave for exg
    unsigned char counter = 0;
    bool positive_square = true;
    bool use_square = false;
    constexpr int num_samples = 256;
    float base_wave[num_samples];
    double amplitude = 1000.0;
    double noise = 0.6;
    double shift = 0.3;

    if (strcmp (params.other_info.c_str (), "square") == 0)
    {
        use_square = true;
    }

    for (int i = 0; i < num_samples; i++)
    {
        float rads = (float)(M_PI / 180.0f);
        base_wave[i] = amplitude * sin (1.8f * i * rads + shift);
    }
    // random distr for exg noise
    uint64_t seed = std::chrono::high_resolution_clock::now ().time_since_epoch ().count ();
    std::mt19937 mt (static_cast<uint32_t> (seed));
    float max_noise = (noise > 0.001f) ? noise : 0.001f;
    float range = (amplitude * max_noise) / 2.0f;
    safe_logger (spdlog::level::info, "noise range is {}:{}", -range, range);
    safe_logger (spdlog::level::info, "amplitude is {}", amplitude);
    safe_logger (spdlog::level::info, "shift is {}", shift);
    std::uniform_real_distribution<float> dist (0 - range, range);

    // get info about synthetic board from json
    int sampling_rate = 250;
    int ec = get_sampling_rate (SYNTHETIC_BOARD, &sampling_rate);
    if (ec != STATUS_OK)
    {
        safe_logger (spdlog::level::warn, "failed to get sampling rate from json: {}", ec);
    }

    double package[SyntheticBoard::package_size] = {0.0};
    int num_exg_channels = 16;

    while (keep_alive)
    {
        package[0] = (double)counter;
        if ((counter % 51 == 0) && (counter != 255))
        {
            positive_square = !positive_square;
        }
        // exg
        for (int i = 0; i < num_exg_channels; i++)
        {
            if (use_square)
            {
                package[i + 1] = positive_square ? amplitude / 2 : -amplitude / 2;
            }
            else
            {
                package[i + 1] = base_wave[counter] + dist (mt);
            }
        }
        // accel
        package[17] = counter / 255.0;
        package[18] = counter / 255.0;
        package[19] = counter / 255.0;
        // gyro
        package[20] = 1.0 - counter / 255.0;
        package[21] = 1.0 - counter / 255.0;
        package[22] = 1.0 - counter / 255.0;
        // eda
        package[23] = amplitude + dist (mt);
        // ppg
        package[24] = 70 + counter / 5.0;
        // temperature
        package[25] = 36 + counter / 200.0;
        // resistance (add just 2 channels)
        package[26] = 1000 + counter / 5.0;
        package[27] = 1000 + counter / 5.0;
        // battery
        package[28] = 100 - counter / 3.0;

        double timestamp = get_timestamp ();
        db->add_data (timestamp, package);
        streamer->stream_data (package, SyntheticBoard::package_size, timestamp);
        counter++;
#ifdef _WIN32
        Sleep ((int)(1000 / sampling_rate));
#else
        usleep ((int)(1000000 / sampling_rate));
#endif
    }
}

int SyntheticBoard::config_board (char *config)
{
    return validate_config (config);
}
