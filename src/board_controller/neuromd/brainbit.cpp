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

#include "cparams.h"
#include "cscanner.h"
#include "sdk_error.h"
#include "stdio.h"


constexpr int BrainBit::package_size;

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


BrainBit::BrainBit (struct BrainFlowInputParams params) : Board ((int)BRAINBIT_BOARD, params)
{
    is_streaming = false;
    keep_alive = false;
    initialized = false;
    device = NULL;
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
        return STATUS_OK;
    }
    if (params.other_info.empty ())
    {
        safe_logger (
            spdlog::level::err, "You need to provide BrainBit serial number to other_info field!");
        return INVALID_ARGUMENTS_ERROR;
    }
    long long serial_number = 0;
    try
    {
        serial_number = std::stoll (params.other_info);
    }
    catch (...)
    {
        safe_logger (
            spdlog::level::err, "You need to provide BrainBit serial number to other_info field!");
        return INVALID_ARGUMENTS_ERROR;
    }
    // try to find device by serial number
    int res = find_device (serial_number);
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
        return GENERAL_ERROR;
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
                        create_EegDoubleChannel_info (device, device_channels.info_array[i]);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "T4") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    signal_t4 =
                        create_EegDoubleChannel_info (device, device_channels.info_array[i]);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "O1") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    signal_o1 =
                        create_EegDoubleChannel_info (device, device_channels.info_array[i]);
                }
            }
            if (strcmp (device_channels.info_array[i].name, "O2") == 0)
            {
                if (brainbit_code == SDK_NO_ERROR)
                {
                    signal_o2 =
                        create_EegDoubleChannel_info (device, device_channels.info_array[i]);
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
        return GENERAL_ERROR;
    }

    free_ChannelInfoArray (device_channels);

    initialized = true;

    return STATUS_OK;
}

int BrainBit::config_board (char *config)
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
    // get Command enum value from value name as char *
    // this enum has more values but looks like they are not supported by brainbit board and used by
    // other devices
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
    if (strcmp (config, "CommandStartResist") == 0)
    {
        parsed = true;
        cmd = CommandStartResist;
    }
    if (strcmp (config, "CommandStopResist") == 0)
    {
        parsed = true;
        cmd = CommandStopResist;
    }

    if (!parsed)
    {
        safe_logger (spdlog::level::err,
            "Invalid value for config, Supported values: CommandStartSignal, CommandStopSignal, "
            "CommandStartResist, CommandStopResist");
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

int BrainBit::start_stream (int buffer_size, char *streamer_params)
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
    db = new DataBuffer (BrainBit::package_size, buffer_size);
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

int BrainBit::stop_stream ()
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

int BrainBit::release_session ()
{
    if (initialized)
    {
        if (is_streaming)
        {
            stop_stream ();
        }
        initialized = false;
        free_listeners ();
        free_channels ();
        free_device ();
    }
    return STATUS_OK;
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
    double package[BrainBit::package_size] = {0.0};
    // I dont see method to flush data from buffer, so need to keep offset and track package num to
    // get only new data
    long long counter = 0;
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
            return;
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

        package[0] = (double)counter;
        package[1] = t3_data * 1e6;
        package[2] = t4_data * 1e6;
        package[3] = o1_data * 1e6;
        package[4] = o2_data * 1e6;
        package[5] = last_resistance_t3;
        package[6] = last_resistance_t4;
        package[7] = last_resistance_o1;
        package[8] = last_resistance_o2;
        package[9] = last_battery;
        db->add_data (timestamp, package);
        streamer->stream_data (package, BrainBit::package_size, timestamp);
    }
}

void BrainBit::free_listeners ()
{
// different headers(free_listener_handle method) for macos and msvc
#ifdef _WIN32
    if (resistance_listener_t4)
    {
        free_listener_handle (resistance_listener_t4);

        resistance_listener_t4 = NULL;
    }
    if (resistance_listener_t3)
    {
        free_listener_handle (resistance_listener_t3);
        resistance_listener_t3 = NULL;
    }
    if (resistance_listener_o1)
    {
        free_listener_handle (resistance_listener_o1);
        resistance_listener_o1 = NULL;
    }
    if (resistance_listener_o2)
    {
        free_listener_handle (resistance_listener_o2);
        resistance_listener_o2 = NULL;
    }
    if (battery_listener)
    {
        free_listener_handle (battery_listener);
        battery_listener = NULL;
    }
#endif
#ifdef APPLE
    if (resistance_listener_t4)
    {
        free_length_listener_handle (resistance_listener_t4);
        resistance_listener_t4 = NULL;
    }
    if (resistance_listener_t3)
    {
        free_length_listener_handle (resistance_listener_t3);
        resistance_listener_t3 = NULL;
    }
    if (resistance_listener_o1)
    {
        free_length_listener_handle (resistance_listener_o1);
        resistance_listener_o1 = NULL;
    }
    if (resistance_listener_o2)
    {
        free_length_listener_handle (resistance_listener_o2);
        resistance_listener_o2 = NULL;
    }
    if (battery_listener)
    {
        free_length_listener_handle (battery_listener);
        battery_listener = NULL;
    }
#endif
}

void BrainBit::free_device ()
{
    if (device)
    {
        device_disconnect (device);
        device_delete (device);
        device = NULL;
    }
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

int BrainBit::find_device (long long serial_number)
{
    DeviceEnumerator *enumerator = create_device_enumerator (DeviceTypeBrainbit);
    if (enumerator == NULL)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, "create enumerator error {}", error_msg);
        return BOARD_NOT_READY_ERROR;
    }

    int attempts = 35;
    int found = false;
    int res = STATUS_OK;
    DeviceInfo device_info;
    do
    {
        if (find_device_info (enumerator, serial_number, &device_info) != 0)
        {
#ifdef _WIN32:
            Sleep (300);
#else
            usleep (300000);
#endif
            continue;
        }

        safe_logger (spdlog::level::info, "Device with SN {} found", serial_number);
        found = true;
        break;
    } while (attempts-- > 0);

    if (found)
    {
        device = create_Device (enumerator, device_info);
        if (device == NULL)
        {
            char error_msg[1024];
            sdk_last_error_msg (error_msg, 1024);
            safe_logger (spdlog::level::err, "create Device error {}", error_msg);
            res = BOARD_NOT_READY_ERROR;
        }
    }
    else
    {
        safe_logger (spdlog::level::err, "Device with SN {} not found", serial_number);
        res = BOARD_NOT_READY_ERROR;
    }

    enumerator_delete (enumerator);

    return res;
}

int BrainBit::find_device_info (
    DeviceEnumerator *enumerator, uint64_t serial_number, DeviceInfo *out_device_info)
{
    DeviceInfoArray device_info_array;
    const int result_code = enumerator_get_device_list (enumerator, &device_info_array);
    if (result_code != SDK_NO_ERROR)
    {
        return -1;
    }

    for (size_t i = 0; i < device_info_array.info_count; ++i)
    {
        if (device_info_array.info_array[i].SerialNumber == serial_number)
        {
            *out_device_info = device_info_array.info_array[i];
            free_DeviceInfoArray (device_info_array);
            return 0;
        }
    }

    free_DeviceInfoArray (device_info_array);
    return -1;
}

int BrainBit::connect_device ()
{
    if (device == NULL)
    {
        safe_logger (spdlog::level::err, "Device is not created.");
        return BOARD_NOT_CREATED_ERROR;
    }

    device_connect (device);

    DeviceState device_state;
    int return_code = device_read_State (device, &device_state);
    if (return_code != SDK_NO_ERROR)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, "device read state error {}", error_msg);
        return BOARD_NOT_READY_ERROR;
    }

    if (device_state != DeviceStateConnected)
    {
        safe_logger (spdlog::level::err, "Device is not connected.");
        return BOARD_NOT_READY_ERROR;
    }

    return STATUS_OK;
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

BrainBit::BrainBit (struct BrainFlowInputParams params) : Board ((int)BRAINBIT_BOARD, params)
{
}

BrainBit::~BrainBit ()
{
}

int BrainBit::prepare_session ()
{
    safe_logger (spdlog::level::err, "BrainBit supports only Windows.");
    return UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::config_board (char *config)
{
    safe_logger (spdlog::level::err, "BrainBit supports only Windows.");
    return UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::release_session ()
{
    safe_logger (spdlog::level::err, "BrainBit supports only Windows.");
    return UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::stop_stream ()
{
    safe_logger (spdlog::level::err, "BrainBit supports only Windows.");
    return UNSUPPORTED_BOARD_ERROR;
}

int BrainBit::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "BrainBit supports only Windows.");
    return UNSUPPORTED_BOARD_ERROR;
}

#endif
