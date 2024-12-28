#include <string>

#include "aavaa_v3.h"
#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"

#define START_BYTE 0xA0
#define END_BYTE 0xC0

#define AAVAA3_WRITE_CHAR "6e400002-c352-11e5-953d-0002a5d5c51b"
#define AAVAA3_NOTIFY_CHAR "6e400003-c352-11e5-953d-0002a5d5c51b"


void aavaa_adapter_1_on_scan_start (simpleble_adapter_t adapter, void *board)
{
    ((AAVAAv3 *)(board))->adapter_1_on_scan_start (adapter);
}

void aavaa_adapter_1_on_scan_stop (simpleble_adapter_t adapter, void *board)
{
    ((AAVAAv3 *)(board))->adapter_1_on_scan_stop (adapter);
}

void aavaa_adapter_1_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((AAVAAv3 *)(board))->adapter_1_on_scan_found (adapter, peripheral);
}

void aavaa_read_notifications (simpleble_peripheral_t handle, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((AAVAAv3 *)(board))->read_data (service, characteristic, data, size);
}

AAVAAv3::AAVAAv3 (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::AAVAA_V3_BOARD, params)
{
    initialized = false;
    aavaa_adapter = NULL;
    aavaa_peripheral = NULL;
    is_streaming = false;
    start_command = "\x01\x62";
    stop_command = "\x01\x39";
}

AAVAAv3::~AAVAAv3 ()
{
    skip_logs = true;
    release_session ();
}

int AAVAAv3::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    if (params.timeout < 1)
    {
        params.timeout = 5;
    }
    safe_logger (spdlog::level::info, "Use timeout for discovery: {}", params.timeout);
    if (!init_dll_loader ())
    {
        safe_logger (spdlog::level::err, "Failed to init dll_loader");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    size_t adapter_count = simpleble_adapter_get_count ();
    if (adapter_count == 0)
    {
        safe_logger (spdlog::level::err, "No BLE adapters found");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    aavaa_adapter = simpleble_adapter_get_handle (0);
    if (aavaa_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    simpleble_adapter_set_callback_on_scan_start (
        aavaa_adapter, ::aavaa_adapter_1_on_scan_start, (void *)this);
    simpleble_adapter_set_callback_on_scan_stop (
        aavaa_adapter, ::aavaa_adapter_1_on_scan_stop, (void *)this);
    simpleble_adapter_set_callback_on_scan_found (
        aavaa_adapter, ::aavaa_adapter_1_on_scan_found, (void *)this);

#ifdef _WIN32
    Sleep (1000);
#else
    sleep (1);
#endif

    if (!simpleble_adapter_is_bluetooth_enabled ())
    {
        safe_logger (spdlog::level::warn, "Probably bluetooth is disabled.");
        // dont throw an exception because of this
        // https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/115
    }

    simpleble_adapter_scan_start (aavaa_adapter);
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, params.timeout * sec, [this] { return this->aavaa_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found AAVAAv3 device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to find AAVAAv3 Device");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (aavaa_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
#ifdef _WIN32
        Sleep (1000);
#else
        sleep (1);
#endif
        // for safety
        for (int i = 0; i < 3; i++)
        {
            if (simpleble_peripheral_connect (aavaa_peripheral) == SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::info, "Connected to AAVAAv3 Device");
                res = (int)BrainFlowExitCodes::STATUS_OK;
                break;
            }
            else
            {
                safe_logger (spdlog::level::err, "Failed to connect to AAVAAv3 Device");
                res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
#ifdef _WIN32
                Sleep (1000);
#else
                sleep (1);
#endif
            }
        }
    }
    else
    {
// https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/26#issuecomment-955606799
#ifdef __linux__
        usleep (1000000);
#endif
    }

    int num_chars_found = 0;

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        size_t services_count = simpleble_peripheral_services_count (aavaa_peripheral);
        for (size_t i = 0; i < services_count; i++)
        {
            simpleble_service_t service;
            if (simpleble_peripheral_services_get (aavaa_peripheral, i, &service) !=
                SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::err, "failed to get service");
                res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
            }

            safe_logger (spdlog::level::trace, "found servce {}", service.uuid.value);
            for (size_t j = 0; j < service.characteristic_count; j++)
            {
                safe_logger (spdlog::level::trace, "found characteristic {}",
                    service.characteristics[j].uuid.value);

                if (strcmp (service.characteristics[j].uuid.value,
                        AAVAA3_WRITE_CHAR) == 0) // Write Characteristics
                {
                    write_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j].uuid);
                    num_chars_found++;
                }
                if (strcmp (service.characteristics[j].uuid.value,
                        AAVAA3_NOTIFY_CHAR) == 0) // Notification Characteristics
                {
                    if (simpleble_peripheral_notify (aavaa_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::aavaa_read_notifications,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                            service.uuid, service.characteristics[j].uuid);
                        num_chars_found++;
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Failed to notify for {} {}",
                            service.uuid.value, service.characteristics[j].uuid.value);
                        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
                    }
                }
            }
        }
    }

    if ((res == (int)BrainFlowExitCodes::STATUS_OK) && (num_chars_found == 2))
    {
        initialized = true;
    }
    else
    {
        release_session ();
    }
    return res;
}

int AAVAAv3::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (is_streaming)
    {
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = send_command (start_command);
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        is_streaming = true;
    }

    return res;
}

int AAVAAv3::stop_stream ()
{
    if (aavaa_peripheral == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (is_streaming)
    {
        res = send_command (stop_command);
    }
    else
    {
        res = (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    }
    is_streaming = false;
    return res;
}

int AAVAAv3::release_session ()
{
    if (initialized)
    {
        // repeat it multiple times, failure here may lead to a crash
        for (int i = 0; i < 2; i++)
        {
            stop_stream ();
            // need to wait for notifications to stop triggered before unsubscribing, otherwise
            // macos fails inside simpleble with timeout
#ifdef _WIN32
            Sleep (1000);
#else
            sleep (1);
#endif
            if (simpleble_peripheral_unsubscribe (aavaa_peripheral, notified_characteristics.first,
                    notified_characteristics.second) != SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::err, "failed to unsubscribe for {} {}",
                    notified_characteristics.first.value, notified_characteristics.second.value);
            }
            else
            {
                safe_logger (spdlog::level::trace, "unsubscribed successfully.");
                break;
            }
        }
        safe_logger (spdlog::level::trace, "freeing packages.");
        free_packages ();
        initialized = false;
    }
    if (aavaa_peripheral != NULL)
    {
        safe_logger (spdlog::level::info, "peripheral is not NULL.");
        bool is_connected = false;
        if (simpleble_peripheral_is_connected (aavaa_peripheral, &is_connected) ==
            SIMPLEBLE_SUCCESS)
        {
            if (is_connected)
            {
                simpleble_peripheral_disconnect (aavaa_peripheral);
            }
        }
        simpleble_peripheral_release_handle (aavaa_peripheral);
        aavaa_peripheral = NULL;
    }
    if (aavaa_adapter != NULL)
    {
        safe_logger (spdlog::level::info, "adapter is not NULL.");
        simpleble_adapter_release_handle (aavaa_adapter);
        aavaa_adapter = NULL;
    }
    safe_logger (spdlog::level::trace, "released successfully.");
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AAVAAv3::config_board (std::string config, std::string &response)
{
    safe_logger (spdlog::level::trace, "config requested: {}", config);
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (config.empty ())
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;

    if (config == "w")
    {
        safe_logger (spdlog::level::trace, "device status was requested {}", device_status);
        response = device_status;
        device_status = "";
        return res;
    }

    if ((config[0] == 'z') || (config[0] == 'Z'))
    {
        bool was_streaming = is_streaming;
        if (was_streaming)
        {
            safe_logger (spdlog::level::trace,
                "disabling streaming to turn on or off impedance, stop command is: {}",
                stop_command.c_str ());
            res = send_command (stop_command);
            if (res == (int)BrainFlowExitCodes::STATUS_OK)
            {
                is_streaming = false;
            }
        }
        if (config[0] == 'z')
        {
            start_command = "z";
            stop_command = "Z";
        }
        else if (config[0] == 'Z')
        {
            start_command = "b";
            stop_command = "s";
        }
        if (was_streaming)
        {
            if (res == (int)BrainFlowExitCodes::STATUS_OK)
            {
                safe_logger (spdlog::level::trace,
                    "enabling streaming to turn on or off impedance, start command is: {}",
                    start_command.c_str ());
                res = send_command (start_command);
            }
            if (res == (int)BrainFlowExitCodes::STATUS_OK)
            {
                is_streaming = true;
            }
        }
    }
    else
    {
        res = send_command (config);
    }
    return res;
}

int AAVAAv3::send_command (std::string config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (config.empty ())
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    uint8_t *command = new uint8_t[config.size ()];
    memcpy (command, config.c_str (), config.size ());
    if (simpleble_peripheral_write_command (aavaa_peripheral, write_characteristics.first,
            write_characteristics.second, command, config.size ()) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        delete[] command;
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    else
    {
        safe_logger (
            spdlog::level::trace, "successfully sent command {} to device", config.c_str ());
    }
    delete[] command;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void AAVAAv3::adapter_1_on_scan_start (simpleble_adapter_t adapter)
{
    safe_logger (spdlog::level::trace, "Scan started");
}

void AAVAAv3::adapter_1_on_scan_stop (simpleble_adapter_t adapter)
{
    safe_logger (spdlog::level::trace, "Scan stopped");
}

void AAVAAv3::adapter_1_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral)
{
    char *peripheral_identified = simpleble_peripheral_identifier (peripheral);
    char *peripheral_address = simpleble_peripheral_address (peripheral);
    bool found = false;
    if (!params.mac_address.empty ())
    {
        if (strcmp (peripheral_address, params.mac_address.c_str ()) == 0)
        {
            found = true;
        }
    }
    else
    {
        if (!params.serial_number.empty ())
        {
            if (strcmp (peripheral_identified, params.serial_number.c_str ()) == 0)
            {
                found = true;
            }
        }
        else
        {
            if (strncmp (peripheral_identified, "AAVAAv3", 8) == 0)
            {
                found = true;
            }
        }
    }

    safe_logger (spdlog::level::trace, "address {}", peripheral_address);
    simpleble_free (peripheral_address);
    safe_logger (spdlog::level::trace, "identifier {}", peripheral_identified);
    simpleble_free (peripheral_identified);

    if (found)
    {
        {
            std::lock_guard<std::mutex> lk (m);
            aavaa_peripheral = peripheral;
        }
        cv.notify_one ();
    }
    else
    {
        simpleble_peripheral_release_handle (peripheral);
    }
}

void AAVAAv3::read_data (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "received {} number of bytes", size);

    if (!is_streaming)
    {
        if (data[1] == 64)
        { // when the first byte is @
            std::string temp (reinterpret_cast<const char *> (data), size);
            device_status = temp.substr (1);
            safe_logger (spdlog::level::trace, "received a status string {} ", device_status);
        }
        else
        {
            safe_logger (spdlog::level::trace, "received a string with start byte {} {} {} {} {}",
                data[0], data[1], data[2], data[3], data[4]);
        }
        return;
    }

    int num_rows = board_descr["default"]["num_rows"];

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];

    if (size == 244)
    {
        data += 3;
        size -= 3;
        Incoming_BLE_Data_Buffer.insert (Incoming_BLE_Data_Buffer.end (), data, data + size);
    }
    else if (size > 1)
    {
        ++data;
        --size;
        Incoming_BLE_Data_Buffer.insert (Incoming_BLE_Data_Buffer.end (), data, data + size);
    }

    while ((int)Incoming_BLE_Data_Buffer.size () >= SIZE_OF_DATA_FRAME)
    {
        if (Incoming_BLE_Data_Buffer[0] != START_BYTE)
        {
            Incoming_BLE_Data_Buffer.pop_front (); // discard data, we have half a frame here
            continue;
        }

        uint8_t *data_frame = new uint8_t[SIZE_OF_DATA_FRAME];
        for (int i = 0; i < SIZE_OF_DATA_FRAME; ++i)
        {
            data_frame[i] = Incoming_BLE_Data_Buffer.front ();
            Incoming_BLE_Data_Buffer.pop_front ();
        }

        if (data_frame[SIZE_OF_DATA_FRAME - 1] != END_BYTE)
        {
            safe_logger (
                spdlog::level::warn, "Wrong End Byte: {}", data_frame[SIZE_OF_DATA_FRAME - 1]);
            continue;
        }

        double *package = new double[num_rows];
        for (int i = 0; i < num_rows; i++)
        {
            package[i] = 0.0;
        }

        // package num
        package[board_descr["default"]["package_num_channel"].get<int> ()] = (double)data_frame[1];

        // eeg
        for (unsigned int i = 0; i < eeg_channels.size (); i++)
        {
            // all the eeg_channels are 4 bytes and we skip the START byte and package number byte
            float f_value;
            std::memcpy (&f_value, data_frame + 2 + 4 * i, sizeof (f_value));
            double d_value = static_cast<double> (f_value);
            package[eeg_channels[i]] = EEG_SCALE * d_value;
        }

        package[board_descr["default"]["rotation_channels"][0].get<int> ()] =
            IMU_SCALE * cast_16bit_to_int32 (data_frame + 34);
        package[board_descr["default"]["rotation_channels"][1].get<int> ()] =
            IMU_SCALE * cast_16bit_to_int32 (data_frame + 36);
        package[board_descr["default"]["rotation_channels"][2].get<int> ()] =
            IMU_SCALE * cast_16bit_to_int32 (data_frame + 38);

        // battery byte
        package[board_descr["default"]["battery_channel"].get<int> ()] = (double)data_frame[40];

        // imu status byte
        package[board_descr["default"]["other_channels"][0].get<int> ()] = (double)data_frame[41];

        // timestamp
        try
        {
            package[board_descr["default"]["other_channels"][1].get<int> ()] =
                *reinterpret_cast<uint32_t *> (data_frame + 42) *
                TIMESTAMP_SCALE; // get_timestamp ();
        }
        catch (const std::exception &e)
        {
            std::string error_message = std::string ("Exception occurred: ") + e.what ();
            safe_logger (spdlog::level::warn, error_message);
        }

        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (package);
        delete[] package;
        delete[] data_frame;
    }
}
