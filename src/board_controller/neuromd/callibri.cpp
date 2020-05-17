#include <stdint.h>
#include <string.h>

#include "callibri.h"

#include "timestamp.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

//////////////////////////////////////////
// Implementation for Windows and APPLE //
//////////////////////////////////////////

#if defined _WIN32 || defined __APPLE__

#include "cparams.h"
#include "cscanner.h"
#include "sdk_error.h"


constexpr int Callibri::package_size;


Callibri::Callibri (int board_id, struct BrainFlowInputParams params)
    : NeuromdBoard (board_id, params)
{
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    signal_channel = NULL;
}

Callibri::~Callibri ()
{
    skip_logs = true;
    release_session ();
}

int Callibri::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }

    // try to find device
    int res = find_device ();
    if (res != STATUS_OK)
    {
        free_device ();
        return res;
    }
    // try to connect to device
    res = connect_device ();
    if (res != STATUS_OK)
    {
        free_device ();
        return res;
    }

    // apply settings for EEG/ECG/EMG
    res = apply_initial_settings ();
    if (res != STATUS_OK)
    {
        free_device ();
        return res;
    }

    ChannelInfoArray device_channels;
    int exit_code = device_available_channels (device, &device_channels);
    if (exit_code != SDK_NO_ERROR)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, "get channels error {}", error_msg);
        free_device ();
        return GENERAL_ERROR;
    }

    for (size_t i = 0; i < device_channels.info_count; ++i)
    {
        if (device_channels.info_array[i].type == ChannelTypeSignal)
        {
            signal_channel =
                create_SignalDoubleChannel_info (device, device_channels.info_array[i]);
        }
    }

    free_ChannelInfoArray (device_channels);

    initialized = true;

    return STATUS_OK;
}

int Callibri::config_board (char *config)
{
    if (device == NULL)
    {
        return BOARD_NOT_CREATED_ERROR;
    }
    if (config == NULL)
    {
        return INVALID_ARGUMENTS_ERROR;
    }

    Command cmd = CommandStartSignal;
    bool parsed = false;
    if (strcmp (config, "CommandStartSignal") == 0)
    {
        parsed = true;
        cmd = CommandStartSignal;
    }
    if (strcmp (config, "CommandStopSignal") == 0)
    {
        parsed = true;
        cmd = CommandStopSignal;
    }
    if (!parsed)
    {
        safe_logger (spdlog::level::err,
            "Invalid value for config, Supported values: CommandStartSignal, CommandStopSignal");
        return INVALID_ARGUMENTS_ERROR;
    }

    int ec = device_execute (device, cmd);
    if (ec != SDK_NO_ERROR)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, error_msg);
        return GENERAL_ERROR;
    }

    return STATUS_OK;
}

int Callibri::start_stream (int buffer_size, char *streamer_params)
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
    db = new DataBuffer (Callibri::package_size, buffer_size);
    if (!db->is_ready ())
    {
        safe_logger (spdlog::level::err, "unable to prepare buffer");
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    res = config_board ((char *)"CommandStartSignal");
    if (res != STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return STATUS_OK;
}

int Callibri::stop_stream ()
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
        int res = config_board ((char *)"CommandStopSignal");
        return res;
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int Callibri::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        initialized = false;
        free_channels ();
        free_device ();
    }
    return STATUS_OK;
}


void Callibri::read_thread ()
{
    double package[Callibri::package_size] = {0.0};
    long long counter = 0;
    while (keep_alive)
    {
        size_t length = 0;

        do
        {
            AnyChannel_get_total_length ((AnyChannel *)signal_channel, &length);
        } while ((keep_alive) && (length < (counter + 1)));

        // check that inner loop ended not because of stop_stream invocation
        if (!keep_alive)
        {
            return;
        }

        double timestamp = get_timestamp ();
        size_t stub = 0;
        double data = 0;
        int sdk_ec = SDK_NO_ERROR;
        sdk_ec =
            DoubleChannel_read_data ((DoubleChannel *)signal_channel, counter, 1, &data, 1, &stub);
        if (sdk_ec != SDK_NO_ERROR)
        {
            continue;
        }
        counter++;

        package[0] = (double)counter;
        package[1] = data * 1e6;
        db->add_data (timestamp, package);
        streamer->stream_data (package, Callibri::package_size, timestamp);
    }
}

void Callibri::free_channels ()
{
    if (signal_channel)
    {
        AnyChannel_delete ((AnyChannel *)signal_channel);
        signal_channel = NULL;
    }
}

////////////////////
// Stub for Linux //
////////////////////

#else

Callibri::Callibri (int board_id, struct BrainFlowInputParams params)
    : NeuromdBoard (board_id, params)
{
}

Callibri::~Callibri ()
{
}

int Callibri::prepare_session ()
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int Callibri::config_board (char *config)
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int Callibri::release_session ()
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int Callibri::stop_stream ()
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int Callibri::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

#endif
