#include <stdint.h>
#include <string.h>

#include "neuromd_board.h"

#include "timestamp.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


NeuromdBoard::NeuromdBoard (int board_id, struct BrainFlowInputParams params)
    : Board (board_id, params)
{
#if defined _WIN32 || defined __APPLE__
    device = NULL;
#endif
}

NeuromdBoard::~NeuromdBoard ()
{
    skip_logs = true;
#if defined _WIN32 || defined __APPLE__
    free_device ();
#endif
}

#if defined _WIN32 || defined __APPLE__

#include "cparams.h"
#include "cscanner.h"
#include "sdk_error.h"


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
        return INVALID_ARGUMENTS_ERROR;
    }

    DeviceEnumerator *enumerator = NULL;
    if (board_id == (int)BRAINBIT_BOARD)
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
        return BOARD_NOT_READY_ERROR;
    }

    int timeout = 15;
    if (params.timeout != 0)
    {
        timeout = params.timeout;
    }
    safe_logger (spdlog::level::info, "set timeout for device discovery to {}", timeout);

    int sleep_delay = 300;
    int attempts = (int)(timeout * 1000.0 / sleep_delay);
    int res = STATUS_OK;
    DeviceInfo device_info;
    do
    {
        res = find_device_info (enumerator, &device_info);
        if (res == INVALID_ARGUMENTS_ERROR)
        {
            break;
        }
        if (res != STATUS_OK)
        {
#ifdef _WIN32:
            Sleep (sleep_delay);
#else
            usleep (sleep_delay * 1000);
#endif
            continue;
        }

        break;
    } while (attempts-- > 0);

    if (res == STATUS_OK)
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
            return INVALID_ARGUMENTS_ERROR;
        }
    }

    int result_code = enumerator_get_device_list (enumerator, &device_info_array);
    if (result_code != SDK_NO_ERROR)
    {
        return GENERAL_ERROR;
    }

    for (size_t i = 0; i < device_info_array.info_count; ++i)
    {
        // here SerialNumber is 0 for Callibri(probably its a bug)
        if (board_id == (int)BRAINBIT_BOARD)
        {
            if ((device_info_array.info_array[i].SerialNumber == serial_number) ||
                (serial_number == 0))
            {
                safe_logger (spdlog::level::info, "Found device with ID {}",
                    device_info_array.info_array[i].SerialNumber);
                *out_device_info = device_info_array.info_array[i];
                free_DeviceInfoArray (device_info_array);
                return STATUS_OK;
            }
        }
        else
        {
            // device_read_SerialNumber returns random string for callibri
            // validate that device name is CallibriYellow and thats all for now
            Device *temp_device = create_Device (enumerator, device_info_array.info_array[i]);
            if (temp_device == NULL)
            {
                safe_logger (spdlog::level::trace, "failed to create device");
                continue;
            }
            char device_name[256];
            result_code = device_read_Name (temp_device, device_name, 256);
            if (result_code != SDK_NO_ERROR)
            {
                safe_logger (spdlog::level::trace, "failed to read device name");
                device_disconnect (temp_device);
                device_delete (temp_device);
                temp_device = NULL;
                continue;
            }
            safe_logger (spdlog::level::info, "device name is {}", device_name);
            device_disconnect (temp_device);
            device_delete (temp_device);
            temp_device = NULL;
            if (strcmp (device_name, "Callibri_Yellow") == 0)
            {
                *out_device_info = device_info_array.info_array[i];
                free_DeviceInfoArray (device_info_array);
                return STATUS_OK;
            }
        }
    }

    free_DeviceInfoArray (device_info_array);
    return BOARD_NOT_READY_ERROR;
}

int NeuromdBoard::connect_device ()
{
    if (device == NULL)
    {
        safe_logger (spdlog::level::err, "Device is not created.");
        return BOARD_NOT_CREATED_ERROR;
    }

#ifndef _WIN32:
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
            return BOARD_NOT_READY_ERROR;
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
        return BOARD_NOT_READY_ERROR;
    }

    return STATUS_OK;
}

#endif
