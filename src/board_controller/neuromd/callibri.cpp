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


Callibri::Callibri (int board_id, struct BrainFlowInputParams params)
    : NeuromdBoard (board_id, params)
{
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    signal_channel = NULL;
    counter = 0;
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
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    // init all function pointers
    int res = NeuromdBoard::prepare_session ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    // try to find device
    res = find_device ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        free_device ();
        return res;
    }
    // try to connect to device
    res = connect_device ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        free_device ();
        return res;
    }

    // apply settings for EEG/ECG/EMG
    res = apply_initial_settings ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
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
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
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
    counter = 0;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Callibri::config_board (std::string config, std::string &response)
{
    if (device == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (config.empty ())
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    Command cmd = CommandStartSignal;
    bool parsed = false;
    if (strcmp (config.c_str (), "CommandStartSignal") == 0)
    {
        parsed = true;
        cmd = CommandStartSignal;
    }
    if (strcmp (config.c_str (), "CommandStopSignal") == 0)
    {
        parsed = true;
        cmd = CommandStopSignal;
    }
    if (!parsed)
    {
        safe_logger (spdlog::level::err,
            "Invalid value for config, Supported values: CommandStartSignal, CommandStopSignal");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int ec = device_execute (device, cmd);
    if (ec != SDK_NO_ERROR)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, error_msg);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Callibri::start_stream (int buffer_size, const char *streamer_params)
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

    std::string command_start = "CommandStartSignal";
    std::string tmp = "";
    res = config_board (command_start, tmp);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Callibri::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        std::string command_stop = "CommandStopSignal";
        std::string tmp = "";
        int res = config_board (command_stop, tmp);
        return res;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
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
        free_packages ();
        initialized = false;
        free_channels ();
    }

    return NeuromdBoard::release_session ();
}


void Callibri::read_thread ()
{
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

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
            break;
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

        package[board_descr["package_num_channel"].get<int> ()] = (double)counter;
        package[1] = data * 1e6; // hardcode channel num here because there are 3 different types
        package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
        push_package (package);
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
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Callibri::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Callibri::release_session ()
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Callibri::stop_stream ()
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int Callibri::start_stream (int buffer_size, const char *streamer_params)
{
    safe_logger (spdlog::level::err, "Callibri doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
