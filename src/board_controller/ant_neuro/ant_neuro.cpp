#include "ant_neuro.h"

// implementation for linux and windows
#if defined(_WIN32) || defined(__linux__) && !defined(__ANDROID__)

#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <chrono>

#include "get_dll_dir.h"
#include "timestamp.h"

#include "eemagine/sdk/factory.h"
#include "eemagine/sdk/wrapper.h"

using namespace eemagine::sdk;


AntNeuroBoard::AntNeuroBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
    // get full path of and neuro library with assumption that this lib is in the same folder
    char ant_neuro_lib_dir[1024];
    bool res = get_dll_path (ant_neuro_lib_dir);
    std::string lib_name = "";
    if (sizeof (void *) == 4)
    {
#ifdef _WIN32
        lib_name = "eego-SDK32.dll";
#else
        lib_name = "libeego-SDK32.so";
#endif
    }
    else
    {
#ifdef _WIN32
        lib_name = "eego-SDK.dll";
#else
        lib_name = "libeego-SDK.so";
#endif
    }
    if (res)
    {
        ant_neuro_lib_path = std::string (ant_neuro_lib_dir) + lib_name;
    }
    else
    {
        ant_neuro_lib_path = lib_name;
    }
    safe_logger (spdlog::level::debug, "use dyn lib: {}", ant_neuro_lib_path.c_str ());

    keep_alive = false;
    initialized = false;
    fact = NULL;
    amp = NULL;
    stream = NULL;
    try
    {
        sampling_rate = board_descr["default"]["sampling_rate"];
    }
    catch (...)
    {
        sampling_rate = -1.0;
    }
    reference_range = -1.0;
    bipolar_range = -1.0;
    impedance_mode = false;
    impedance_package_num = -1;
}

AntNeuroBoard::~AntNeuroBoard ()
{
    skip_logs = true;
    release_session ();
}

int AntNeuroBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    try
    {
        fact = new factory (ant_neuro_lib_path);
        safe_logger (spdlog::level::info, "eego sdk version is: {}.{}.{}.{}",
            fact->getVersion ().major, fact->getVersion ().minor, fact->getVersion ().micro,
            fact->getVersion ().build);
        amp = fact->getAmplifier ();
        reference_range = amp->getReferenceRangesAvailable ()[0];
        bipolar_range = amp->getBipolarRangesAvailable ()[0];
        if (sampling_rate < 0)
        {
            sampling_rate = amp->getSamplingRatesAvailable ()[0];
        }
        impedance_mode = false;
        impedance_package_num = 0;
    }
    catch (const exceptions::notFound &e)
    {
        safe_logger (spdlog::level::err, "No devices found, {}", e.what ());
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    catch (const std::runtime_error &e)
    {
        safe_logger (spdlog::level::err, "Exception, {}. Lib path {}", e.what (),
            ant_neuro_lib_path.c_str ());
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    catch (...)
    {
        safe_logger (
            spdlog::level::err, "Failed to create factory from {}", ant_neuro_lib_path.c_str ());
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    initialized = true;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroBoard::start_stream (int buffer_size, const char *streamer_params)
{
    if (stream != NULL)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    if (amp == NULL)
    {
        safe_logger (spdlog::level::err, "Amplifier is not created");
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    try
    {
        if (impedance_mode)
        {
            safe_logger (spdlog::level::info, "start impedance stream");
            stream = amp->OpenImpedanceStream ();
        }
        else
        {
            safe_logger (spdlog::level::info,
                "start eeg stream (sampling rate: {}, reference range: {}, bipolar range: {})",
                sampling_rate, reference_range, bipolar_range);
            stream = amp->OpenEegStream (sampling_rate, reference_range, bipolar_range);
        }
    }
    catch (const std::runtime_error &e)
    {
        safe_logger (spdlog::level::err, "Failed to start acquisition: {}", e.what ());
        return (int)BrainFlowExitCodes::STREAM_THREAD_ERROR;
    }
    if (stream == NULL)
    {
        safe_logger (spdlog::level::err, "Failed to start acquisition.");
        return (int)BrainFlowExitCodes::STREAM_THREAD_ERROR;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AntNeuroBoard::stop_stream ()
{
    if (stream != NULL)
    {
        keep_alive = false;
        streaming_thread.join ();
        delete stream;
        stream = NULL;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else
    {
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int AntNeuroBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
        initialized = false;
    }
    if (amp != NULL)
    {
        delete amp;
        amp = NULL;
    }
    if (fact != NULL)
    {
        delete fact;
        fact = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void AntNeuroBoard::read_thread ()
{
    if ((amp == NULL) || (stream == NULL))
    {
        safe_logger (spdlog::level::err, "amp or stream not created in thread");
        return;
    }

    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }
    std::vector<int> emg_channels;
    std::vector<int> eeg_channels;
    std::vector<int> resistance_channels;
    try
    {
        emg_channels = board_descr["default"]["emg_channels"].get<std::vector<int>> ();
    }
    catch (...)
    {
        safe_logger (spdlog::level::trace, "device has no emg channels");
    }
    try
    {
        eeg_channels = board_descr["default"]["eeg_channels"].get<std::vector<int>> ();
    }
    catch (...)
    {
        safe_logger (spdlog::level::trace, "device has no eeg channels");
    }
    try
    {
        resistance_channels =
            board_descr["default"]["resistance_channels"].get<std::vector<int>> ();
    }
    catch (...)
    {
        safe_logger (spdlog::level::trace, "device has no resistance_channels channels");
    }
    std::vector<channel> ant_channels = stream->getChannelList ();

    while (keep_alive)
    {
        try
        {
            buffer buf = stream->getData ();
            int buf_channels_len = buf.getChannelCount ();
            for (int i = 0; i < (int)buf.getSampleCount (); i++)
            {
                int eeg_counter = 0;
                int emg_counter = 0;
                int resistance_counter = 0;
                for (int j = 0; j < buf_channels_len; j++)
                {
                    if ((ant_channels[j].getType () == channel::reference) &&
                        (eeg_counter < (int)eeg_channels.size ()))
                    {
                        package[eeg_channels[eeg_counter++]] = buf.getSample (j, i);
                        if (impedance_mode)
                        {
                            resistance_counter++;
                        }
                    }
                    if ((ant_channels[j].getType () == channel::bipolar) &&
                        (emg_counter < (int)emg_channels.size ()))
                    {
                        package[emg_channels[emg_counter++]] = buf.getSample (j, i);
                        if (impedance_mode)
                        {
                            resistance_counter++;
                        }
                    }
                    if (ant_channels[j].getType () == channel::sample_counter)
                    {
                        package[board_descr["default"]["package_num_channel"].get<int> ()] =
                            buf.getSample (j, i);
                    }
                    if (ant_channels[j].getType () == channel::trigger)
                    {
                        package[board_descr["default"]["other_channels"][0].get<int> ()] =
                            buf.getSample (j, i);
                    }
                    if ((ant_channels[j].getType () == channel::impedance_reference) &&
                        (resistance_counter < (int)resistance_channels.size ()))
                    {
                        package[resistance_channels[resistance_counter++]] = buf.getSample (j, i);
                    }
                    if ((ant_channels[j].getType () == channel::impedance_ground) &&
                        (resistance_counter < (int)resistance_channels.size ()))
                    {
                        package[resistance_channels[resistance_counter++]] = buf.getSample (j, i);
                    }
                }
                package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
                if (impedance_mode)
                {
                    package[board_descr["default"]["package_num_channel"].get<int> ()] =
                        impedance_package_num++;
                }
                push_package (package);
            }
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
            if (impedance_mode)
            {
                // some more sleep; twice every second should be more than enough
                // if left out, it yields impedances at around 64 Hz
                std::this_thread::sleep_for (std::chrono::milliseconds (500));
            }
        }
        catch (...)
        {
            safe_logger (spdlog::level::err, "exception in data thread.");
            std::this_thread::sleep_for (std::chrono::milliseconds (100));
        }
    }
    delete[] package;
}

int AntNeuroBoard::config_board (std::string config, std::string &response)
{
    if (amp == NULL)
    {
        safe_logger (spdlog::level::err, "Amplifier is not created");
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    std::string prefix = "sampling_rate:";
    std::string rv_prefix = "reference_range:";
    std::string bv_prefix = "bipolar_range:";
    std::string mode_prefix = "impedance_mode:";

    if (config.find (prefix) != std::string::npos)
    {
        int new_sampling_rate = 0;
        std::string value = config.substr (prefix.size ());
        try
        {
            new_sampling_rate = std::stoi (value);
        }
        catch (...)
        {
            safe_logger (spdlog::level::err, "format is '{}value'", prefix.c_str ());
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        // check that provided value is correct
        std::vector<int> allowed_values = amp->getSamplingRatesAvailable ();
        if (std::find (allowed_values.begin (), allowed_values.end (), new_sampling_rate) !=
            allowed_values.end ())
        {
            sampling_rate = new_sampling_rate;
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        else
        {
            safe_logger (spdlog::level::err, "not supported value provided");
            for (int i = 0; i < (int)allowed_values.size (); i++)
            {
                safe_logger (spdlog::level::debug, "supported value: {}", allowed_values[i]);
            }
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    else if (config.find (rv_prefix) != std::string::npos)
    {
        double new_reference_range = 0.0;
        std::string value = config.substr (rv_prefix.size ());
        try
        {
            new_reference_range = std::stod (value);
        }
        catch (...)
        {
            safe_logger (spdlog::level::err, "format is '{}value'", rv_prefix.c_str ());
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        // check that provided value is correct
        std::vector<double> allowed_values = amp->getReferenceRangesAvailable ();
        if (std::find (allowed_values.begin (), allowed_values.end (), new_reference_range) !=
            allowed_values.end ())
        {
            reference_range = new_reference_range;
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        else
        {
            safe_logger (spdlog::level::err, "not supported value provided");
            for (int i = 0; i < (int)allowed_values.size (); i++)
            {
                safe_logger (spdlog::level::debug, "supported value: {}", allowed_values[i]);
            }
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }

        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else if (config.find (bv_prefix) != std::string::npos)
    {
        double new_bipolar_range = 0.0;
        std::string value = config.substr (bv_prefix.size ());
        try
        {
            new_bipolar_range = std::stod (value);
        }
        catch (...)
        {
            safe_logger (spdlog::level::err, "format is '{}value'", bv_prefix.c_str ());
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        // check that provided value is correct
        std::vector<double> allowed_values = amp->getBipolarRangesAvailable ();
        if (std::find (allowed_values.begin (), allowed_values.end (), new_bipolar_range) !=
            allowed_values.end ())
        {
            bipolar_range = new_bipolar_range;
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        else
        {
            safe_logger (spdlog::level::err, "not supported value provided");
            for (int i = 0; i < (int)allowed_values.size (); i++)
            {
                safe_logger (spdlog::level::debug, "supported value: {}", allowed_values[i]);
            }
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }

        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    else if (config.find (mode_prefix) != std::string::npos)
    {
        bool new_impedance_mode;
        std::string value = config.substr (mode_prefix.size ());

        if (value == "0" || value == "1")
        {
            try
            {
                new_impedance_mode = static_cast<bool> (std::stod (value));
            }
            catch (...)
            {
                safe_logger (spdlog::level::err, "format is '{}value'", mode_prefix.c_str ());
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }

            impedance_mode = new_impedance_mode;
            return (int)BrainFlowExitCodes::STATUS_OK;
        }
        else
        {
            safe_logger (spdlog::level::err, "not supported value provided");
            safe_logger (spdlog::level::debug, "supported values: '0' or '1'");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    safe_logger (spdlog::level::err, "format is '{}value'", prefix.c_str ());
    return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
}

// stub for macos
#else
AntNeuroBoard::AntNeuroBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
}

AntNeuroBoard::~AntNeuroBoard ()
{
}

int AntNeuroBoard::prepare_session ()
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::release_session ()
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::stop_stream ()
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int AntNeuroBoard::start_stream (int buffer_size, const char *streamer_params)
{
    safe_logger (spdlog::level::err, "AntNeuroBoard doesnt support MacOS.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
#endif
