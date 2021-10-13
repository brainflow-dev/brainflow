#include <stdint.h>
#include <string.h>

#include "brainbit.h"

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


namespace BrainBitCallbacks
{
    // convert plain C callbacks to class methods
    void on_battery_charge_received (
        Device *device, ChannelInfo channel_info, IntDataArray battery_data, void *user_data)
    {
        ((BrainBit *)user_data)->on_battery_charge_received (device, channel_info, battery_data);
    }

    void on_resistance_received (
        Device *device, ChannelInfo channel_info, DoubleDataArray data_array, void *user_data)
    {
        ((BrainBit *)user_data)->on_resistance_received (device, channel_info, data_array);
    }
}


BrainBit::BrainBit (struct BrainFlowInputParams params)
    : NeuromdBoard ((int)BoardIds::BRAINBIT_BOARD, params)
{
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    last_battery = -1;
    last_resistance_t4 = 0.0;
    last_resistance_t3 = 0.0;
    last_resistance_o2 = 0.0;
    last_resistance_o1 = 0.0;
    battery_listener = NULL;
    signal_t4 = NULL;
    signal_t3 = NULL;
    signal_o1 = NULL;
    signal_o2 = NULL;
    counter = 0;
}

BrainBit::~BrainBit ()
{
    skip_logs = true;
    release_session ();
}

int BrainBit::prepare_session ()
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

    // need to set callbacks for battery charge and resistance data
    // get info about channels
    ChannelInfoArray device_channels;
    int brainbit_code = device_available_channels (device, &device_channels);
    if (brainbit_code != SDK_NO_ERROR)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, "get channels error {}", error_msg);
        free_device ();
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    // set correct callbacks for channels
    for (size_t i = 0; i < device_channels.info_count; ++i)
    {
        //////////////////////////
        //////// Battery /////////
        //////////////////////////
        if (device_channels.info_array[i].type == ChannelTypeBattery)
        {
            if (brainbit_code == SDK_NO_ERROR)
            {
                brainbit_code = device_subscribe_int_channel_data_received (device,
                    device_channels.info_array[i], &BrainBitCallbacks::on_battery_charge_received,
                    &battery_listener, (void *)this);
            }
        }
        ////////////////////
        //// Resistance ////
        ////////////////////
        if (device_channels.info_array[i].type == ChannelTypeResistance)
        {
            if (strcmp (device_channels.info_array[i].name, "T3") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    brainbit_code = device_subscribe_double_channel_data_received (device,
                        device_channels.info_array[i], &BrainBitCallbacks::on_resistance_received,
                        &resistance_listener_t3, (void *)this);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "T4") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    brainbit_code = device_subscribe_double_channel_data_received (device,
                        device_channels.info_array[i], &BrainBitCallbacks::on_resistance_received,
                        &resistance_listener_t4, (void *)this);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "O1") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    brainbit_code = device_subscribe_double_channel_data_received (device,
                        device_channels.info_array[i], &BrainBitCallbacks::on_resistance_received,
                        &resistance_listener_o1, (void *)this);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "O2") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    brainbit_code = device_subscribe_double_channel_data_received (device,
                        device_channels.info_array[i], &BrainBitCallbacks::on_resistance_received,
                        &resistance_listener_o2, (void *)this);
                }
            }
        }
        ////////////////////
        //// EEG Signal ////
        ////////////////////
        if (device_channels.info_array[i].type == ChannelTypeSignal)
        {
            if (strcmp (device_channels.info_array[i].name, "T3") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {

                    signal_t3 =
                        create_SignalDoubleChannel_info (device, device_channels.info_array[i]);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "T4") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    signal_t4 =
                        create_SignalDoubleChannel_info (device, device_channels.info_array[i]);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "O1") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    signal_o1 =
                        create_SignalDoubleChannel_info (device, device_channels.info_array[i]);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "O2") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    signal_o2 =
                        create_SignalDoubleChannel_info (device, device_channels.info_array[i]);
                }
            }
        }
    }
    if (brainbit_code != SDK_NO_ERROR)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, "get channels error {}", error_msg);
        free_listeners ();
        free_channels ();
        free_device ();
        free_ChannelInfoArray (device_channels);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    free_ChannelInfoArray (device_channels);

    initialized = true;
    counter = 0;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainBit::config_board (std::string config, std::string &response)
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
    // get Command enum value from value name as char *
    // this enum has more values but looks like they are not supported by brainbit board and used by
    // other devices
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
    if (strcmp (config.c_str (), "CommandStartResist") == 0)
    {
        parsed = true;
        cmd = CommandStartResist;
    }
    if (strcmp (config.c_str (), "CommandStopResist") == 0)
    {
        parsed = true;
        cmd = CommandStopResist;
    }

    if (!parsed)
    {
        safe_logger (spdlog::level::err,
            "Invalid value for config, Supported values: CommandStartSignal, CommandStopSignal, "
            "CommandStartResist, CommandStopResist");
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

int BrainBit::start_stream (int buffer_size, const char *streamer_params)
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

int BrainBit::stop_stream ()
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

int BrainBit::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        free_packages ();
        initialized = false;
        free_listeners ();
        free_channels ();
    }
    return NeuromdBoard::release_session ();
}

void BrainBit::read_thread ()
{
    /*
     * Format for output from BrainFlow is:
     * package[0] - package counter
     * package [1-4] - eeg t3, t4, o1, o2
     * package[5-8] - resistance t3, t4, o1, o2. Place it to other_channels
     * package[9] - battery
     */
    int num_rows = board_descr["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    while (keep_alive)
    {
        size_t length_t3 = 0;
        size_t length_t4 = 0;
        size_t length_o1 = 0;
        size_t length_o2 = 0;

        do
        {
            AnyChannel_get_total_length ((AnyChannel *)signal_t3, &length_t3);
            AnyChannel_get_total_length ((AnyChannel *)signal_t4, &length_t4);
            AnyChannel_get_total_length ((AnyChannel *)signal_o1, &length_o1);
            AnyChannel_get_total_length ((AnyChannel *)signal_o2, &length_o2);
        } while ((keep_alive) &&
            ((length_o2 < (counter + 1) || length_o1 < (counter + 1) || length_t3 < (counter + 1) ||
                length_t4 < (counter + 1))));

        // check that inner loop ended not because of stop_stream invocation
        if (!keep_alive)
        {
            break;
        }

        // get timestamp as soon as loop ends
        double timestamp = get_timestamp ();
        double t3_data;
        double t4_data;
        double o1_data;
        double o2_data;

        size_t stub = 0;
        int sdk_ec = SDK_NO_ERROR;
        sdk_ec =
            DoubleChannel_read_data ((DoubleChannel *)signal_t3, counter, 1, &t3_data, 1, &stub);
        if (sdk_ec != SDK_NO_ERROR)
        {
            continue;
        }
        sdk_ec =
            DoubleChannel_read_data ((DoubleChannel *)signal_t4, counter, 1, &t4_data, 1, &stub);
        if (sdk_ec != SDK_NO_ERROR)
        {
            continue;
        }
        sdk_ec =
            DoubleChannel_read_data ((DoubleChannel *)signal_o1, counter, 1, &o1_data, 1, &stub);
        if (sdk_ec != SDK_NO_ERROR)
        {
            continue;
        }
        sdk_ec =
            DoubleChannel_read_data ((DoubleChannel *)signal_o2, counter, 1, &o2_data, 1, &stub);
        if (sdk_ec != SDK_NO_ERROR)
        {
            continue;
        }
        counter++;

        package[board_descr["package_num_channel"].get<int> ()] = (double)counter;
        package[board_descr["eeg_channels"][0].get<int> ()] = t3_data * 1e6;
        package[board_descr["eeg_channels"][1].get<int> ()] = t4_data * 1e6;
        package[board_descr["eeg_channels"][2].get<int> ()] = o1_data * 1e6;
        package[board_descr["eeg_channels"][3].get<int> ()] = o2_data * 1e6;
        package[board_descr["resistance_channels"][0].get<int> ()] = last_resistance_t3;
        package[board_descr["resistance_channels"][1].get<int> ()] = last_resistance_t4;
        package[board_descr["resistance_channels"][2].get<int> ()] = last_resistance_o1;
        package[board_descr["resistance_channels"][3].get<int> ()] = last_resistance_o2;
        package[board_descr["battery_channel"].get<int> ()] = last_battery;
        package[board_descr["timestamp_channel"].get<int> ()] = timestamp;
        push_package (package);
    }
}

void BrainBit::free_listeners ()
{
    free_listener (resistance_listener_t4);
    free_listener (resistance_listener_t3);
    free_listener (resistance_listener_o1);
    free_listener (resistance_listener_o2);
    free_listener (battery_listener);
}

void BrainBit::free_channels ()
{
    if (signal_t4)
    {
        AnyChannel_delete ((AnyChannel *)signal_t4);
        signal_t4 = NULL;
    }
    if (signal_t3)
    {
        AnyChannel_delete ((AnyChannel *)signal_t3);
        signal_t3 = NULL;
    }
    if (signal_o1)
    {
        AnyChannel_delete ((AnyChannel *)signal_o1);
        signal_o1 = NULL;
    }
    if (signal_o2)
    {
        AnyChannel_delete ((AnyChannel *)signal_o2);
        signal_o2 = NULL;
    }
}

void BrainBit::on_battery_charge_received (
    Device *device, ChannelInfo channel_info, IntDataArray battery_data)
{
    if (battery_data.samples_count > 0)
    {
        last_battery = battery_data.data_array[battery_data.samples_count - 1];
    }
    free_IntDataArray (battery_data);
}

void BrainBit::on_resistance_received (
    Device *device, ChannelInfo channel_info, DoubleDataArray resistance_array)
{
    // no mutex here, so its not atomic operation but I think its ok
    // dont want to add mutex since I am not sure that underlying lib will not hang somewhere
    if (channel_info.type == ChannelTypeResistance)
    {
        if (resistance_array.samples_count > 0)
        {
            if (strcmp (channel_info.name, "T3") == 0)
            {
                last_resistance_t3 =
                    resistance_array.data_array[resistance_array.samples_count - 1];
            }
            if (strcmp (channel_info.name, "T4") == 0)
            {
                last_resistance_t4 =
                    resistance_array.data_array[resistance_array.samples_count - 1];
            }
            if (strcmp (channel_info.name, "O1") == 0)
            {
                last_resistance_o1 =
                    resistance_array.data_array[resistance_array.samples_count - 1];
            }
            if (strcmp (channel_info.name, "O2") == 0)
            {
                last_resistance_o2 =
                    resistance_array.data_array[resistance_array.samples_count - 1];
            }
        }
    }
    free_DoubleDataArray (resistance_array);
}

////////////////////
// Stub for Linux //
////////////////////

#else

BrainBit::BrainBit (struct BrainFlowInputParams params)
    : NeuromdBoard ((int)BoardIds::BRAINBIT_BOARD, params)
{
}

BrainBit::~BrainBit ()
{
}

int BrainBit::prepare_session ()
{
    safe_logger (spdlog::level::err, "BrainBit doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::err, "BrainBit doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::release_session ()
{
    safe_logger (spdlog::level::err, "BrainBit doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::stop_stream ()
{
    safe_logger (spdlog::level::err, "BrainBit doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::start_stream (int buffer_size, const char *streamer_params)
{
    safe_logger (spdlog::level::err, "BrainBit doesnt support Linux.");
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
