#include "unicorn_board.h"

// implementation for linux
#ifdef __linux__

#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "get_dll_dir.h"
#include "timestamp.h"
#include "unicorn_board.h"


constexpr int UnicornBoard::package_size;


UnicornBoard::UnicornBoard (struct BrainFlowInputParams params) : Board ((int)UNICORN_BOARD, params)
{
    // get full path of libunicorn.so with assumption that this lib is in the same folder
    char unicornlib_dir[1024];
    device_handle = 0;
    bool res = get_dll_path (unicornlib_dir);
    std::string unicornlib_path = "";
    if (res)
    {
        unicornlib_path = std::string (unicornlib_dir) + "libunicorn.so";
    }
    else
    {
        unicornlib_path = "libunicorn.so";
    }

    safe_logger (spdlog::level::debug, "use dyn lib: {}", unicornlib_path.c_str ());
    dll_loader = new DLLLoader (unicornlib_path.c_str ());

    is_streaming = false;
    keep_alive = false;
    initialized = false;
    func_get_data = NULL;
}

UnicornBoard::~UnicornBoard ()
{
    skip_logs = true;
    release_session ();
}

int UnicornBoard::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return STATUS_OK;
    }

    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        return GENERAL_ERROR;
    }
    safe_logger (spdlog::level::debug, "Library is loaded");

    func_get_data = (int (*) (UNICORN_HANDLE, uint32_t, float *, uint32_t))dll_loader->get_address (
        "UNICORN_GetData");
    if (func_get_data == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for UNICORN_GetData");
        return GENERAL_ERROR;
    }

    int res = call_open ();
    if (res != STATUS_OK)
    {
        return res;
    }

    initialized = true;
    return STATUS_OK;
}

int UnicornBoard::start_stream (int buffer_size, char *streamer_params)
{
    if (is_streaming)
    {
        safe_logger (spdlog::level::err, "Streaming thread already running");
        return STREAM_ALREADY_RUN_ERROR;
    }
    if (buffer_size <= 0 || buffer_size > MAX_CAPTURE_SAMPLES)
    {
        safe_logger (spdlog::level::err, "Invalid array size");
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

    db = new DataBuffer (UnicornBoard::package_size, buffer_size);
    if (!db->is_ready ())
    {
        Board::board_logger->error ("Unable to prepare buffer with size {}", buffer_size);
        delete db;
        db = NULL;
        return INVALID_BUFFER_SIZE_ERROR;
    }

    res = call_start ();
    if (res != STATUS_OK)
    {
        return res;
    }

    keep_alive = true;
    streaming_thread = std::thread ([this] { this->read_thread (); });
    is_streaming = true;
    return STATUS_OK;
}

int UnicornBoard::stop_stream ()
{
    if (is_streaming)
    {
        keep_alive = false;
        is_streaming = false;
        streaming_thread.join ();
        return call_stop ();
    }
    else
    {
        return STREAM_THREAD_IS_NOT_RUNNING;
    }
}

int UnicornBoard::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        initialized = false;
    }
    if (dll_loader != NULL)
    {
        func_get_data = NULL;
        call_close ();
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }
    return STATUS_OK;
}

void UnicornBoard::read_thread ()
{
    double package[UnicornBoard::package_size];
    float temp_buffer[UnicornBoard::package_size];

    while (keep_alive)
    {
        // unicorn uses similar idea as in brainflow - return single array with different kinds of
        // data and provide API(defines in this case) to mark this data
        func_get_data (device_handle, 1, temp_buffer, UnicornBoard::package_size * sizeof (float));
        double timestamp = get_timestamp ();
        // eeg data
        package[0] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX];
        package[1] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 1];
        package[2] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 2];
        package[3] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 3];
        package[4] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 4];
        package[5] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 5];
        package[6] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 6];
        package[7] = (double)temp_buffer[UNICORN_EEG_CONFIG_INDEX + 7];
        // accel data
        package[8] = (double)temp_buffer[UNICORN_ACCELEROMETER_CONFIG_INDEX];
        package[9] = (double)temp_buffer[UNICORN_ACCELEROMETER_CONFIG_INDEX + 1];
        package[10] = (double)temp_buffer[UNICORN_ACCELEROMETER_CONFIG_INDEX + 2];
        // gyro data
        package[11] = (double)temp_buffer[UNICORN_GYROSCOPE_CONFIG_INDEX];
        package[12] = (double)temp_buffer[UNICORN_GYROSCOPE_CONFIG_INDEX + 1];
        package[13] = (double)temp_buffer[UNICORN_GYROSCOPE_CONFIG_INDEX + 2];
        // battery data
        package[14] = (double)temp_buffer[UNICORN_BATTERY_CONFIG_INDEX];
        // counter / package num
        package[15] = (double)temp_buffer[UNICORN_COUNTER_CONFIG_INDEX];
        // validation config index? place it to other channels
        package[16] = (double)temp_buffer[UNICORN_VALIDATION_CONFIG_INDEX];

        db->add_data (timestamp, package);
        streamer->stream_data (package, UnicornBoard::package_size, timestamp);
    }
}

int UnicornBoard::config_board (char *config)
{
    // todo if there will be requests for it.
    // Unicorn API provides int Unicorn_SetConfiguration method
    safe_logger (spdlog::level::debug, "config_board is not supported for Unicorn.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::call_open ()
{
    // find devices
    int (*func_get_available) (UNICORN_DEVICE_SERIAL *, uint32_t *, BOOL) =
        (int (*) (UNICORN_DEVICE_SERIAL *, uint32_t *, BOOL))dll_loader->get_address (
            "UNICORN_GetAvailableDevices");
    if (func_get_available == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for UNICORN_GetAvailableDevices");
        return GENERAL_ERROR;
    }
    unsigned int available_device_count = 0;
    int ec = func_get_available (NULL, &available_device_count, TRUE);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_GetAvailableDevices {}", ec);
        return BOARD_NOT_READY_ERROR;
    }
    if (available_device_count < 1)
    {
        safe_logger (spdlog::level::err, "Unicorn not found");
        return BOARD_NOT_READY_ERROR;
    }
    UNICORN_DEVICE_SERIAL *available_devices = new UNICORN_DEVICE_SERIAL[available_device_count];
    ec = func_get_available (available_devices, &available_device_count, TRUE);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_GetAvailableDevices {}", ec);
        delete[] available_devices;
        return BOARD_NOT_READY_ERROR;
    }

    // search for device
    int device_num = 0;
    if (params.serial_number.empty ())
    {
        safe_logger (spdlog::level::warn,
            "Use device with id {}. To select another one provide id to serial_number field.",
            available_devices[device_num]);
    }
    else
    {
        for (device_num = 0; device_num < available_device_count; device_num++)
        {
            if (strcmp (available_devices[device_num], params.serial_number.c_str ()) == 0)
            {
                break;
            }
        }
        if (device_num == available_device_count)
        {
            safe_logger (
                spdlog::level::err, "device with id {} not found", params.serial_number.c_str ());
            delete[] available_devices;
            return GENERAL_ERROR;
        }
    }

    // open device
    int (*func_open) (UNICORN_DEVICE_SERIAL, UNICORN_HANDLE *) = (int (*) (
        UNICORN_DEVICE_SERIAL, UNICORN_HANDLE *))dll_loader->get_address ("UNICORN_OpenDevice");
    if (func_open == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for UNICORN_OpenDevice");
        delete[] available_devices;
        return GENERAL_ERROR;
    }
    ec = func_open (available_devices[device_num], &device_handle);
    if ((ec != UNICORN_ERROR_SUCCESS) || (device_handle == 0))
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_OpenDevice {}", ec);
        delete[] available_devices;
        return BOARD_NOT_READY_ERROR;
    }

    delete[] available_devices;
    return STATUS_OK;
}

int UnicornBoard::call_start ()
{
    int (*func_start_streaming) (UNICORN_HANDLE, BOOL) =
        (int (*) (UNICORN_HANDLE, BOOL))dll_loader->get_address ("UNICORN_StartAcquisition");
    if (func_start_streaming == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for UNICORN_StartAcquisition");
        return GENERAL_ERROR;
    }
    int ec = func_start_streaming (device_handle, FALSE);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_StartAcquisition {}", ec);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int UnicornBoard::call_stop ()
{
    int (*func_stop_streaming) (UNICORN_HANDLE) =
        (int (*) (UNICORN_HANDLE))dll_loader->get_address ("UNICORN_StopAcquisition");
    if (func_stop_streaming == NULL)
    {
        safe_logger (
            spdlog::level::err, "failed to get function address for UNICORN_StopAcquisition");
        return GENERAL_ERROR;
    }
    int ec = func_stop_streaming (device_handle);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_StopAcquisition {}", ec);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}

int UnicornBoard::call_close ()
{
    int (*func_close) (UNICORN_HANDLE *) =
        (int (*) (UNICORN_HANDLE *))dll_loader->get_address ("UNICORN_CloseDevice");
    if (func_close == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for UNICORN_CloseDevice");
        return GENERAL_ERROR;
    }
    int ec = func_close (&device_handle);
    if (ec != UNICORN_ERROR_SUCCESS)
    {
        safe_logger (spdlog::level::err, "Error in UNICORN_CloseDevice {}", ec);
        return GENERAL_ERROR;
    }
    return STATUS_OK;
}


// stub for windows and macos
#else
UnicornBoard::UnicornBoard (struct BrainFlowInputParams params) : Board ((int)UNICORN_BOARD, params)
{
}

UnicornBoard::~UnicornBoard ()
{
}

int UnicornBoard::prepare_session ()
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::config_board (char *config)
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::release_session ()
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::stop_stream ()
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}

int UnicornBoard::start_stream (int buffer_size, char *streamer_params)
{
    safe_logger (spdlog::level::err, "UnicornBoard supports only Linux.");
    return UNSUPPORTED_BOARD_ERROR;
}
#endif
