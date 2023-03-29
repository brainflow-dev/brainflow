#include <string>

#include "custom_cast.h"
#include "aavaa_3c2.h"
#include "get_dll_dir.h"
#include "timestamp.h"

#define START_BYTE 0xA0
#define END_BYTE_STANDARD 0xC0
#define END_BYTE_ANALOG 0xC1
#define END_BYTE_MAX 0xC6

#define AAVAA3_WRITE_CHAR "6e400002-c352-11e5-953d-0002a5d5c51b"
#define AAVAA3_NOTIFY_CHAR "6e400003-c352-11e5-953d-0002a5d5c51b"


static void aavaa_adapter_1_on_scan_start (simpleble_adapter_t adapter, void *board)
{
    ((AAVAA3c2 *)(board))->adapter_1_on_scan_start (adapter);
}

static void aavaa_adapter_1_on_scan_stop (simpleble_adapter_t adapter, void *board)
{
    ((AAVAA3c2 *)(board))->adapter_1_on_scan_stop (adapter);
}

static void aavaa_adapter_1_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((AAVAA3c2 *)(board))->adapter_1_on_scan_found (adapter, peripheral);
}

static void aavaa_read_notifications (simpleble_uuid_t service, simpleble_uuid_t characteristic,
    uint8_t *data, size_t size, void *board)
{
    ((AAVAA3c2 *)(board))->read_data (service, characteristic, data, size);
}

AAVAA3c2::AAVAA3c2 (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::AAVAA_3C2_BOARD, params)
{
    initialized = false;
    aavaa_adapter = NULL;
    aavaa_peripheral = NULL;
    is_streaming = false;
    start_command = "\x01\x62";
    stop_command = "\x01\x39";
}

AAVAA3c2::~AAVAA3c2 ()
{
    skip_logs = true;
    release_session ();
}

int AAVAA3c2::prepare_session ()
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
    usleep (1000000);
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
    if (cv.wait_for (
            lk, params.timeout * sec, [this] { return this->aavaa_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found AAVAA3c2 device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to find AAVAA3c2 Device");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (aavaa_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (simpleble_peripheral_connect (aavaa_peripheral) == SIMPLEBLE_SUCCESS)
        {
            safe_logger (spdlog::level::info, "Connected to AAVAA3c2 Device");
        }
        else
        {
            safe_logger (spdlog::level::err, "Failed to connect to AAVAA3c2 Device");
            res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
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

int AAVAA3c2::start_stream (int buffer_size, const char *streamer_params)
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

int AAVAA3c2::stop_stream ()
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

int AAVAA3c2::release_session ()
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
            Sleep (2000);
#else
            sleep (2);
#endif
            if (simpleble_peripheral_unsubscribe (aavaa_peripheral,
                    notified_characteristics.first,
                    notified_characteristics.second) != SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::err, "failed to unsubscribe for {} {}",
                    notified_characteristics.first.value, notified_characteristics.second.value);
            }
            else
            {
                break;
            }
        }
        free_packages ();
        initialized = false;
    }
    if (aavaa_peripheral != NULL)
    {
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
        simpleble_adapter_release_handle (aavaa_adapter);
        aavaa_adapter = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int AAVAA3c2::config_board (std::string config, std::string &response)
{
    return config_board (config);
}

int AAVAA3c2::config_board (std::string config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (config.empty ())
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;
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

int AAVAA3c2::send_command (std::string config)
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
        safe_logger (spdlog::level::trace, "successfully sent command {} to device", config.c_str ());
    }
    delete[] command;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void AAVAA3c2::adapter_1_on_scan_start (simpleble_adapter_t adapter)
{
    safe_logger (spdlog::level::trace, "Scan started");
}

void AAVAA3c2::adapter_1_on_scan_stop (simpleble_adapter_t adapter)
{
    safe_logger (spdlog::level::trace, "Scan stopped");
}

void AAVAA3c2::adapter_1_on_scan_found (
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
            if (strncmp (peripheral_identified, "AAVAA3c2", 8) == 0)
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

void AAVAA3c2::read_data (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    if (!is_streaming)
    {
        return;
    }
    safe_logger (spdlog::level::trace, "received {} number of bytes", size);

    int num_rows = board_descr["default"]["num_rows"];

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    double accel_scale = (double)(0.002 / (pow (2, 4)));

    if (size == 244) {
        data += 3;
        size -= 3;
        Incoming_BLE_Data_Buffer.insert(Incoming_BLE_Data_Buffer.end(), data, data + size);
    } else if (size == 48) {
        ++data;
        --size;
        Incoming_BLE_Data_Buffer.insert(Incoming_BLE_Data_Buffer.end(), data, data + size);
    }

    while (Incoming_BLE_Data_Buffer.size() >= SIZE_OF_DATA_FRAME) {
        if (Incoming_BLE_Data_Buffer[0] != START_BYTE) {
            Incoming_BLE_Data_Buffer.pop_front(); // discard data, we have half a frame here
            continue;
        }

        uint8_t* data_frame = new uint8_t[SIZE_OF_DATA_FRAME];
        for (int i = 0; i < SIZE_OF_DATA_FRAME; ++i) {
            data_frame[i] = Incoming_BLE_Data_Buffer.front();
            Incoming_BLE_Data_Buffer.pop_front();
        }

        double accel[3] = {0.};
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
            double eeg_scale = (double)(4.5 / float ((pow (2, 23) - 1)) /
                gain_tracker.get_gain_for_channel (i) * 1000000.);
            package[eeg_channels[i]] = eeg_scale * cast_24bit_to_int32 (data_frame + 2 + 3 * i);
        }
        // end byte
        package[board_descr["default"]["other_channels"][0].get<int> ()] = (double)data_frame[32];
        // place unprocessed bytes for all modes to other_channels
        package[board_descr["default"]["other_channels"][1].get<int> ()] = (double)data_frame[26];
        package[board_descr["default"]["other_channels"][2].get<int> ()] = (double)data_frame[27];
        package[board_descr["default"]["other_channels"][3].get<int> ()] = (double)data_frame[28];
        package[board_descr["default"]["other_channels"][4].get<int> ()] = (double)data_frame[29];
        package[board_descr["default"]["other_channels"][5].get<int> ()] = (double)data_frame[30];
        package[board_descr["default"]["other_channels"][6].get<int> ()] = (double)data_frame[31];
        // place processed bytes for accel
        if (data_frame[SIZE_OF_DATA_FRAME-1] == END_BYTE_STANDARD)
        {
            int32_t accel_temp[3] = {0};
            accel_temp[0] = cast_16bit_to_int32 (data_frame + 26);
            accel_temp[1] = cast_16bit_to_int32 (data_frame + 28);
            accel_temp[2] = cast_16bit_to_int32 (data_frame + 30);

            if (accel_temp[0] != 0)
            {
                accel[0] = accel_scale * accel_temp[0];
                accel[1] = accel_scale * accel_temp[1];
                accel[2] = accel_scale * accel_temp[2];
            }

            package[board_descr["default"]["accel_channels"][0].get<int> ()] = accel[0];
            package[board_descr["default"]["accel_channels"][1].get<int> ()] = accel[1];
            package[board_descr["default"]["accel_channels"][2].get<int> ()] = accel[2];
        }

        // place processed bytes for analog
        if (data_frame[SIZE_OF_DATA_FRAME-1] == END_BYTE_ANALOG)
        {
            package[board_descr["default"]["analog_channels"][0].get<int> ()] =
                cast_16bit_to_int32 (data_frame + 26);
            package[board_descr["default"]["analog_channels"][1].get<int> ()] =
                cast_16bit_to_int32 (data_frame + 28);
            package[board_descr["default"]["analog_channels"][2].get<int> ()] =
                cast_16bit_to_int32 (data_frame + 30);
        }

        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (package);
        delete[] package;
        delete[] data_frame;
    }
}
