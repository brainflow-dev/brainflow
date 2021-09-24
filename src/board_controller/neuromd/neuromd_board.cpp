#include <stdint.h>
#include <string.h>

#include "get_dll_dir.h"
#include "neuromd_board.h"
#include "timestamp.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


///////////////////////////////////////
/// Windows or Apple Implementation ///
///////////////////////////////////////

#if defined _WIN32 || defined __APPLE__


NeuromdBoard::NeuromdBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
    device = NULL;

    std::string neurosdk_path = "";
    std::string neurosdk_name = "";
    char neurosdk_dir[1024];
    bool res = get_dll_path (neurosdk_dir);
#ifdef _WIN32
    if (sizeof (void *) == 4)
    {
        neurosdk_name = "neurosdk-x86.dll";
    }
    else
    {
        neurosdk_name = "neurosdk-x64.dll";
    }
#endif
#ifdef __APPLE__
    neurosdk_name = "libneurosdk-shared.dylib";
#endif

    if (res)
    {
        neurosdk_path = std::string (neurosdk_dir) + neurosdk_name;
    }
    else
    {
        neurosdk_path = neurosdk_name;
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", neurosdk_path.c_str ());
    dll_loader = new DLLLoader (neurosdk_path.c_str ());

    sdk_last_error_msg = NULL;
    device_disconnect = NULL;
    device_delete = NULL;
    create_device_enumerator = NULL;
    create_Device = NULL;
    enumerator_delete = NULL;
    enumerator_get_device_list = NULL;
    free_DeviceInfoArray = NULL;
    device_read_Name = NULL;
    device_read_SerialNumber = NULL;
    device_connect = NULL;
    device_read_State = NULL;
    device_set_SamplingFrequency = NULL;
    device_set_Gain = NULL;
    device_set_Offset = NULL;
    device_set_ExternalSwitchState = NULL;
    device_set_ADCInputState = NULL;
    device_set_HardwareFilterState = NULL;
    device_available_channels = NULL;
    create_SignalDoubleChannel_info = NULL;
    free_ChannelInfoArray = NULL;
    device_execute = NULL;
    AnyChannel_get_total_length = NULL;
    DoubleChannel_read_data = NULL;
    AnyChannel_delete = NULL;
    device_subscribe_int_channel_data_received = NULL;
    device_subscribe_double_channel_data_received = NULL;
    free_IntDataArray = NULL;
    free_DoubleDataArray = NULL;
#ifdef _WIN32
    free_listener_handle = NULL;
#else
    free_length_listener_handle = NULL;
#endif
}

NeuromdBoard::~NeuromdBoard ()
{
    skip_logs = true;
    release_session ();
}

int NeuromdBoard::prepare_session ()
{
    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    // Find addresses for all methods which we will use

    // sdk_last_error_msg
    sdk_last_error_msg = (int (*) (char *, size_t))dll_loader->get_address ("sdk_last_error_msg");
    if (sdk_last_error_msg == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for sdk_last_error_msg");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_disconnect
    device_disconnect = (int (*) (Device *))dll_loader->get_address ("device_disconnect");
    if (device_disconnect == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_disconnect");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_delete
    device_delete = (void (*) (Device *))dll_loader->get_address ("device_delete");
    if (device_delete == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_delete");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // create_device_enumerator
    create_device_enumerator =
        (DeviceEnumerator * (*)(DeviceType)) dll_loader->get_address ("create_device_enumerator");
    if (create_device_enumerator == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for create_device_enumerator");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // create_Device
    create_Device =
        (Device * (*)(DeviceEnumerator *, DeviceInfo)) dll_loader->get_address ("create_Device");
    if (create_Device == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for create_Device");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // enumerator_delete
    enumerator_delete =
        (void (*) (DeviceEnumerator *))dll_loader->get_address ("enumerator_delete");
    if (enumerator_delete == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for enumerator_delete");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // enumerator_get_device_list
    enumerator_get_device_list = (int (*) (DeviceEnumerator *,
        DeviceInfoArray *))dll_loader->get_address ("enumerator_get_device_list");
    if (enumerator_get_device_list == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for enumerator_get_device_list");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // free_DeviceInfoArray
    free_DeviceInfoArray =
        (void (*) (DeviceInfoArray))dll_loader->get_address ("free_DeviceInfoArray");
    if (free_DeviceInfoArray == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for free_DeviceInfoArray");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_read_Name
    device_read_Name =
        (int (*) (Device *, char *, size_t))dll_loader->get_address ("device_read_Name");
    if (device_read_Name == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_read_Name");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_read_SerialNumber
    device_read_SerialNumber =
        (int (*) (Device *, char *, size_t))dll_loader->get_address ("device_read_SerialNumber");
    if (device_read_SerialNumber == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for device_read_SerialNumber");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_connect
    device_connect = (int (*) (Device *))dll_loader->get_address ("device_connect");
    if (device_connect == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_connect");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_read_State
    device_read_State =
        (int (*) (Device *, DeviceState *))dll_loader->get_address ("device_read_State");
    if (device_read_State == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_read_State");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_set_SamplingFrequency
    device_set_SamplingFrequency = (int (*) (Device *, SamplingFrequency))dll_loader->get_address (
        "device_set_SamplingFrequency");
    if (device_set_SamplingFrequency == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for device_set_SamplingFrequency");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_set_Gain
    device_set_Gain = (int (*) (Device *, Gain))dll_loader->get_address ("device_set_Gain");
    if (device_set_Gain == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_set_Gain");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_set_Offset
    device_set_Offset =
        (int (*) (Device *, unsigned char))dll_loader->get_address ("device_set_Offset");
    if (device_set_Offset == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_set_Offset");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_set_ExternalSwitchState
    device_set_ExternalSwitchState = (int (*) (
        Device *, ExternalSwitchInput))dll_loader->get_address ("device_set_ExternalSwitchState");
    if (device_set_ExternalSwitchState == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for device_set_ExternalSwitchState");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_set_ADCInputState
    device_set_ADCInputState =
        (int (*) (Device *, ADCInput))dll_loader->get_address ("device_set_ADCInputState");
    if (device_set_ADCInputState == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for device_set_ADCInputState");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_set_HardwareFilterState
    device_set_HardwareFilterState =
        (int (*) (Device *, bool))dll_loader->get_address ("device_set_HardwareFilterState");
    if (device_set_HardwareFilterState == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for device_set_HardwareFilterState");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_available_channels
    device_available_channels = (int (*) (
        const Device *, ChannelInfoArray *))dll_loader->get_address ("device_available_channels");
    if (device_available_channels == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for device_available_channels");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // create_SignalDoubleChannel_info
    create_SignalDoubleChannel_info = (SignalDoubleChannel *
        (*)(Device *, ChannelInfo)) dll_loader->get_address ("create_SignalDoubleChannel_info");
    if (create_SignalDoubleChannel_info == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for create_SignalDoubleChannel_info");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // free_ChannelInfoArray
    free_ChannelInfoArray =
        (void (*) (ChannelInfoArray))dll_loader->get_address ("free_ChannelInfoArray");
    if (free_ChannelInfoArray == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for free_ChannelInfoArray");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_execute
    device_execute = (int (*) (Device *, Command))dll_loader->get_address ("device_execute");
    if (device_execute == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for device_execute");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // AnyChannel_get_total_length
    AnyChannel_get_total_length =
        (int (*) (AnyChannel *, size_t *))dll_loader->get_address ("AnyChannel_get_total_length");
    if (AnyChannel_get_total_length == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for AnyChannel_get_total_length");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // DoubleChannel_read_data
    DoubleChannel_read_data = (int (*) (DoubleChannel *, size_t, size_t, double *, size_t,
        size_t *))dll_loader->get_address ("DoubleChannel_read_data");
    if (DoubleChannel_read_data == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for DoubleChannel_read_data");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // AnyChannel_delete
    AnyChannel_delete = (void (*) (AnyChannel *))dll_loader->get_address ("AnyChannel_delete");
    if (AnyChannel_delete == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for AnyChannel_delete");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_subscribe_int_channel_data_received
    device_subscribe_int_channel_data_received = (int (*) (Device *, ChannelInfo,
        void (*) (Device *, ChannelInfo, IntDataArray, void *), ListenerHandle *,
        void *))dll_loader->get_address ("device_subscribe_int_channel_data_received");
    if (device_subscribe_int_channel_data_received == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for device_subscribe_int_channel_data_received");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // device_subscribe_double_channel_data_received
    device_subscribe_double_channel_data_received = (int (*) (Device *, ChannelInfo,
        void (*) (Device *, ChannelInfo, DoubleDataArray, void *), ListenerHandle *,
        void *))dll_loader->get_address ("device_subscribe_double_channel_data_received");
    if (device_subscribe_double_channel_data_received == NULL)
    {
        safe_logger (spdlog::level::err,
            "failed to get function address for device_subscribe_double_channel_data_received");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // free_IntDataArray
    free_IntDataArray = (void (*) (IntDataArray))dll_loader->get_address ("free_IntDataArray");
    if (free_IntDataArray == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for free_IntDataArray");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    // free_DoubleDataArray
    free_DoubleDataArray =
        (void (*) (DoubleDataArray))dll_loader->get_address ("free_DoubleDataArray");
    if (free_DoubleDataArray == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for free_DoubleDataArray");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
#ifdef _WIN32
    // free_listener_handle
    free_listener_handle =
        (void (*) (ListenerHandle))dll_loader->get_address ("free_listener_handle");
    if (free_listener_handle == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for free_listener_handle");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
#else
    // free_length_listener_handle
    free_length_listener_handle =
        (void (*) (LengthListenerHandle))dll_loader->get_address ("free_length_listener_handle");
    if (free_length_listener_handle == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for free_length_listener_handle");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
#endif

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int NeuromdBoard::release_session ()
{
    free_device ();
    if (dll_loader != NULL)
    {
        delete dll_loader;
        dll_loader = NULL;
    }

    sdk_last_error_msg = NULL;
    device_disconnect = NULL;
    device_delete = NULL;
    create_device_enumerator = NULL;
    create_Device = NULL;
    enumerator_delete = NULL;
    enumerator_get_device_list = NULL;
    free_DeviceInfoArray = NULL;
    device_read_Name = NULL;
    device_connect = NULL;
    device_read_State = NULL;
    device_set_SamplingFrequency = NULL;
    device_set_Gain = NULL;
    device_set_Offset = NULL;
    device_set_ExternalSwitchState = NULL;
    device_set_ADCInputState = NULL;
    device_set_HardwareFilterState = NULL;
    device_available_channels = NULL;
    create_SignalDoubleChannel_info = NULL;
    free_ChannelInfoArray = NULL;
    device_execute = NULL;
    AnyChannel_get_total_length = NULL;
    DoubleChannel_read_data = NULL;
    AnyChannel_delete = NULL;
    device_subscribe_int_channel_data_received = NULL;
    device_subscribe_double_channel_data_received = NULL;
    free_IntDataArray = NULL;
    free_DoubleDataArray = NULL;
#ifdef _WIN32
    free_listener_handle = NULL;
#else
    free_length_listener_handle = NULL;
#endif

    return (int)BrainFlowExitCodes::STATUS_OK;
}

void NeuromdBoard::free_listener (ListenerHandle lh)
{
    if (lh)
    {
        // different headers for macos and msvc
#ifdef _WIN32
        free_listener_handle (lh);
#else
        free_length_listener_handle (lh);
#endif
        lh = NULL;
    }
}

void NeuromdBoard::free_device ()
{
    if (device)
    {
        device_disconnect (device);
        device_delete (device);
        device = NULL;
    }
}

int NeuromdBoard::find_device ()
{
    if ((params.timeout < 0) || (params.timeout > 600))
    {
        safe_logger (spdlog::level::err, "bad value for timeout");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    DeviceEnumerator *enumerator = NULL;
    if (board_id == (int)BoardIds::BRAINBIT_BOARD)
    {
        enumerator = create_device_enumerator (DeviceTypeBrainbit);
    }
    else
    {
        enumerator = create_device_enumerator (DeviceTypeCallibri);
    }
    if (enumerator == NULL)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::err, "create enumerator error {}", error_msg);
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    int timeout = 15;
    if (params.timeout != 0)
    {
        timeout = params.timeout;
    }
    safe_logger (spdlog::level::info, "set timeout for device discovery to {}", timeout);

    int sleep_delay = 300;
    int attempts = (int)(timeout * 1000.0 / sleep_delay);
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    DeviceInfo device_info;
    do
    {
        res = find_device_info (enumerator, &device_info);
        if (res == (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR)
        {
            break;
        }
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
#ifdef _WIN32
            Sleep (sleep_delay);
#else
            usleep (sleep_delay * 1000);
#endif
            continue;
        }

        break;
    } while (attempts-- > 0);

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        device = create_Device (enumerator, device_info);
        if (device == NULL)
        {
            char error_msg[1024];
            sdk_last_error_msg (error_msg, 1024);
            safe_logger (spdlog::level::err, "create Device error {}", error_msg);
            res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }

    enumerator_delete (enumerator);

    return res;
}

int NeuromdBoard::find_device_info (DeviceEnumerator *enumerator, DeviceInfo *out_device_info)
{
    DeviceInfoArray device_info_array;
    long long serial_number = 0;
    if (!params.serial_number.empty ())
    {
        try
        {
            serial_number = std::stoll (params.serial_number);
        }
        catch (...)
        {
            safe_logger (spdlog::level::err,
                "You need to provide NeuromdBoard serial number to serial_number field!");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    int result_code = enumerator_get_device_list (enumerator, &device_info_array);
    if (result_code != SDK_NO_ERROR)
    {
        char error_msg[1024];
        sdk_last_error_msg (error_msg, 1024);
        safe_logger (spdlog::level::warn, "enumerator_get_device_list error: {}", error_msg);
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    for (size_t i = 0; i < device_info_array.info_count; ++i)
    {
        // here SerialNumber is 0 for Callibri(probably its a bug)
        if (board_id == (int)BoardIds::BRAINBIT_BOARD)
        {
            if ((device_info_array.info_array[i].SerialNumber == serial_number) ||
                (serial_number == 0))
            {
                safe_logger (spdlog::level::info, "Found device with ID {}",
                    device_info_array.info_array[i].SerialNumber);
                *out_device_info = device_info_array.info_array[i];
                free_DeviceInfoArray (device_info_array);
                return (int)BrainFlowExitCodes::STATUS_OK;
            }
        }
        // its very different for callibri and brainbit, maybe in future versions of neurosdk will
        // be similar
        else
        {
            Device *temp_device = create_Device (enumerator, device_info_array.info_array[i]);
            if (temp_device == NULL)
            {
                safe_logger (spdlog::level::trace, "failed to create device");
                continue;
            }
            // need to connect to read serial number
            device_connect (temp_device);
            DeviceState device_state = DeviceStateDisconnected;
            for (int i = 0; (i < 3) && (device_state != DeviceStateConnected); i++)
            {
                int return_code = device_read_State (temp_device, &device_state);
                if (return_code != SDK_NO_ERROR)
                {
                    break;
                }
#ifdef _WIN32
                Sleep (1000);
#else
                usleep (1000000);
#endif
            }
            if (device_state != DeviceStateConnected)
            {
                device_delete (temp_device);
                temp_device = NULL;
                continue;
            }
            // now can read device name and serial number
            char device_name[256];
            char device_num[256];
            result_code = device_read_Name (temp_device, device_name, 256);
            if (result_code != SDK_NO_ERROR)
            {
                safe_logger (spdlog::level::trace, "failed to read device name");
                device_disconnect (temp_device);
                device_delete (temp_device);
                temp_device = NULL;
                continue;
            }
            result_code = device_read_SerialNumber (temp_device, device_num, 256);
            if (result_code != SDK_NO_ERROR)
            {
                safe_logger (spdlog::level::trace, "failed to read device number");
                device_disconnect (temp_device);
                device_delete (temp_device);
                temp_device = NULL;
                continue;
            }
            safe_logger (spdlog::level::info, "device name is {}, Serial number is {}", device_name,
                device_num);
            device_disconnect (temp_device);
            device_delete (temp_device);
            temp_device = NULL;
            if (strcmp (device_name, "Callibri_Yellow") == 0)
            {
                // for callibri its a string
                if (!params.serial_number.empty ())
                {
                    if (strcmp (device_num, params.serial_number.c_str ()) == 0)
                    {
                        *out_device_info = device_info_array.info_array[i];
                        free_DeviceInfoArray (device_info_array);
                        return (int)BrainFlowExitCodes::STATUS_OK;
                    }
                    else
                    {
                        safe_logger (
                            spdlog::level::info, "device doesn't match provided serial number");
                    }
                }
                else
                {
                    *out_device_info = device_info_array.info_array[i];
                    free_DeviceInfoArray (device_info_array);
                    return (int)BrainFlowExitCodes::STATUS_OK;
                }
            }
        }
    }

    free_DeviceInfoArray (device_info_array);
    return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
}

int NeuromdBoard::connect_device ()
{
    if (device == NULL)
    {
        safe_logger (spdlog::level::err, "Device is not created.");
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

#ifndef _WIN32
    // "Fix" a race condition in BrainBit SDK on MacOS
    usleep (5000000);
#endif

    device_connect (device);

    DeviceState device_state = DeviceStateDisconnected;
    // on Callibri first attemp fails, repeat several times
    for (int i = 0; (i < 5) && (device_state != DeviceStateConnected); i++)
    {
        int return_code = device_read_State (device, &device_state);
        if (return_code != SDK_NO_ERROR)
        {
            char error_msg[1024];
            sdk_last_error_msg (error_msg, 1024);
            safe_logger (spdlog::level::err, "device read state error {}", error_msg);
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
#ifdef _WIN32
        Sleep (1000);
#else
        usleep (1000000);
#endif
    }

    if (device_state != DeviceStateConnected)
    {
        safe_logger (spdlog::level::err, "Device is not connected.");
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

////////////////////////////
/// Linux Implementation ///
////////////////////////////

#else

NeuromdBoard::NeuromdBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
}
NeuromdBoard::~NeuromdBoard ()
{
}
int NeuromdBoard::prepare_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
int NeuromdBoard::release_session ()
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}

#endif
