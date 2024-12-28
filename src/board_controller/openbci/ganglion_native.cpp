#include <string>

#include "custom_cast.h"
#include "ganglion_native.h"
#include "get_dll_dir.h"
#include "timestamp.h"


#define GANGLION_WRITE_CHAR "2d30c083-f39f-4ce6-923f-3484ea480596"
#define GANGLION_NOTIFY_CHAR "2d30c082-f39f-4ce6-923f-3484ea480596"
#define GANGLION_SOFTWARE_REVISION "00002a28-0000-1000-8000-00805f9b34fb"


static void ganglion_adapter_1_on_scan_start (simpleble_adapter_t adapter, void *board)
{
    ((GanglionNative *)(board))->adapter_1_on_scan_start (adapter);
}

static void ganglion_adapter_1_on_scan_stop (simpleble_adapter_t adapter, void *board)
{
    ((GanglionNative *)(board))->adapter_1_on_scan_stop (adapter);
}

static void ganglion_adapter_1_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((GanglionNative *)(board))->adapter_1_on_scan_found (adapter, peripheral);
}

static void ganglion_read_notifications (simpleble_peripheral_t handle, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((GanglionNative *)(board))->read_data (service, characteristic, data, size);
}

GanglionNative::GanglionNative (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::GANGLION_NATIVE_BOARD, params)
{
    initialized = false;
    ganglion_adapter = NULL;
    ganglion_peripheral = NULL;
    is_streaming = false;
    start_command = "b";
    stop_command = "s";
    firmware = 0;
}

GanglionNative::~GanglionNative ()
{
    skip_logs = true;
    release_session ();
}

int GanglionNative::prepare_session ()
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

    ganglion_adapter = simpleble_adapter_get_handle (0);
    if (ganglion_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    simpleble_adapter_set_callback_on_scan_start (
        ganglion_adapter, ::ganglion_adapter_1_on_scan_start, (void *)this);
    simpleble_adapter_set_callback_on_scan_stop (
        ganglion_adapter, ::ganglion_adapter_1_on_scan_stop, (void *)this);
    simpleble_adapter_set_callback_on_scan_found (
        ganglion_adapter, ::ganglion_adapter_1_on_scan_found, (void *)this);

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

    simpleble_adapter_scan_start (ganglion_adapter);
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (
            lk, params.timeout * sec, [this] { return this->ganglion_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found GanglionNative device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to find Ganglion Device");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (ganglion_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // for safety
        for (int i = 0; i < 3; i++)
        {
            if (simpleble_peripheral_connect (ganglion_peripheral) == SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::info, "Connected to Ganglion Device");
                res = (int)BrainFlowExitCodes::STATUS_OK;
                break;
            }
            else
            {
                safe_logger (spdlog::level::warn, "Failed to connect to Ganglion Device: {}/3", i);
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
        size_t services_count = simpleble_peripheral_services_count (ganglion_peripheral);
        for (size_t i = 0; i < services_count; i++)
        {
            simpleble_service_t service;
            if (simpleble_peripheral_services_get (ganglion_peripheral, i, &service) !=
                SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::err, "failed to get service");
                res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
            }

            safe_logger (spdlog::level::trace, "found service {}", service.uuid.value);
            for (size_t j = 0; j < service.characteristic_count; j++)
            {
                // Read the software revision characteristic to get the firmware version
                if (strcmp (service.characteristics[j].uuid.value, GANGLION_SOFTWARE_REVISION) == 0)
                {
                    uint8_t *data;
                    size_t data_length;
                    simpleble_peripheral_read (ganglion_peripheral, service.uuid,
                        service.characteristics[j].uuid, &data, &data_length);

                    // Data should be in the form x.x.x stored as ASCII values in the data buffer
                    firmware = (data[0] == '3') ? 3 : 2;
                    safe_logger (spdlog::level::info, "Detected firmware version {}", firmware);
                }

                if (strcmp (service.characteristics[j].uuid.value,
                        GANGLION_WRITE_CHAR) == 0) // Write Characteristics
                {
                    write_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j].uuid);
                    num_chars_found++;
                }
                if (strcmp (service.characteristics[j].uuid.value,
                        GANGLION_NOTIFY_CHAR) == 0) // Notification Characteristics
                {
                    if (simpleble_peripheral_notify (ganglion_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::ganglion_read_notifications,
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

int GanglionNative::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    if (is_streaming)
    {
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    temp_data.reset (); // reset last data before streaming
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

int GanglionNative::stop_stream ()
{
    if (ganglion_peripheral == NULL)
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

int GanglionNative::release_session ()
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
            if (simpleble_peripheral_unsubscribe (ganglion_peripheral,
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
    if (ganglion_peripheral != NULL)
    {
        bool is_connected = false;
        if (simpleble_peripheral_is_connected (ganglion_peripheral, &is_connected) ==
            SIMPLEBLE_SUCCESS)
        {
            if (is_connected)
            {
                simpleble_peripheral_disconnect (ganglion_peripheral);
            }
        }
        simpleble_peripheral_release_handle (ganglion_peripheral);
        ganglion_peripheral = NULL;
    }
    if (ganglion_adapter != NULL)
    {
        simpleble_adapter_release_handle (ganglion_adapter);
        ganglion_adapter = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int GanglionNative::config_board (std::string config, std::string &response)
{
    // todo do smth to unify available commands
    // and make them more or less standard between devices
    if (config == "get_firmware_version")
    {
        response = std::to_string (firmware);
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    return config_board (config);
}

int GanglionNative::config_board (std::string config)
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

int GanglionNative::send_command (std::string config)
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
    if (simpleble_peripheral_write_command (ganglion_peripheral, write_characteristics.first,
            write_characteristics.second, command, config.size ()) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        delete[] command;
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    delete[] command;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void GanglionNative::adapter_1_on_scan_start (simpleble_adapter_t adapter)
{
    safe_logger (spdlog::level::trace, "Scan started");
}

void GanglionNative::adapter_1_on_scan_stop (simpleble_adapter_t adapter)
{
    safe_logger (spdlog::level::trace, "Scan stopped");
}

void GanglionNative::adapter_1_on_scan_found (
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
            if (strncmp (peripheral_identified, "Ganglion", 8) == 0)
            {
                found = true;
            }
            // for some reason device may send Simblee instead Ganglion name
            else if (strncmp (peripheral_identified, "Simblee", 7) == 0)
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
            ganglion_peripheral = peripheral;
        }
        cv.notify_one ();
    }
    else
    {
        simpleble_peripheral_release_handle (peripheral);
    }
}

void GanglionNative::read_data (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    if (size < 2)
    {
        safe_logger (spdlog::level::warn, "unexpected number of bytes received: {}", size);
        return;
    }

    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];

    for (int i = 0; i < num_rows; i++)
    {
        package[i] = 0.0;
    }

    if (data[0] <= 200 && size == 20)
    {
        if (firmware == 3)
        {
            decompress_firmware_3 (data, package);
        }
        else if (firmware == 2)
        {
            decompress_firmware_2 (data, package);
        }
        delete[] package;
    }
    else if ((data[0] > 200) && (data[0] < 206))
    {
        // ASCII string with value and 'Z' in the end
        int val = 0;
        int i = 0;
        for (i = 1; i < 6; i++)
        {
            if (data[i] == 'Z')
            {
                break;
            }
        }
        std::string ascii_value ((const char *)(data + 1), i - 1);

        try
        {
            val = std::stoi (ascii_value);
        }
        catch (...)
        {
            safe_logger (
                spdlog::level::err, "failed to parse impedance data: {}", ascii_value.c_str ());
            delete[] package;
            return;
        }

        switch (data[0] % 10)
        {
            case 1:
                temp_data.resist_first = val;
                break;
            case 2:
                temp_data.resist_second = val;
                break;
            case 3:
                temp_data.resist_third = val;
                break;
            case 4:
                temp_data.resist_fourth = val;
                break;
            case 5:
                temp_data.resist_ref = val;
                break;
            default:
                break;
        }
        package[board_descr["default"]["package_num_channel"].get<int> ()] = data[0];
        package[board_descr["default"]["resistance_channels"][0].get<int> ()] =
            temp_data.resist_first;
        package[board_descr["default"]["resistance_channels"][1].get<int> ()] =
            temp_data.resist_second;
        package[board_descr["default"]["resistance_channels"][2].get<int> ()] =
            temp_data.resist_third;
        package[board_descr["default"]["resistance_channels"][3].get<int> ()] =
            temp_data.resist_fourth;
        package[board_descr["default"]["resistance_channels"][4].get<int> ()] =
            temp_data.resist_ref;
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
        delete[] package;
        return;
    }
    else
    {
        for (int i = 0; i < 20; i++)
        {
            safe_logger (spdlog::level::warn, "byte {} value {}", i, data[i]);
        }
        delete[] package;
        return;
    }
}

void GanglionNative::decompress_firmware_3 (const uint8_t *data, double *package)
{
    int bits_per_num = 0;
    unsigned char package_bits[160] = {0}; // 20 * 8
    for (int i = 0; i < 20; i++)
    {
        uchar_to_bits (data[i], package_bits + i * 8);
    }

    // 18 bit compression, sends 17 MSBs + sign bit of 24-bit sample
    if (data[0] >= 0 && data[0] < 100)
    {
        int last_digit = data[0] % 10;
        switch (last_digit)
        {
            // accel data is signed, so we must cast it to signed char
            // swap x and z, and invert z to convert to standard coordinate space.
            case 0:
                temp_data.accel_z = -accel_scale * (char)data[19];
                break;
            case 1:
                temp_data.accel_y = accel_scale * (char)data[19];
                break;
            case 2:
                temp_data.accel_x = accel_scale * (char)data[19];
                break;
            default:
                break;
        }
        bits_per_num = 18;
    }
    else if (data[0] >= 100 && data[0] < 200)
    {
        bits_per_num = 19;
    }

    // handle compressed data for 18 or 19 bits
    for (int i = 8, counter = 0; i < bits_per_num * 8; i += bits_per_num, counter++)
    {
        if (bits_per_num == 18)
        {
            temp_data.last_data[counter] =
                (float)(cast_ganglion_bits_to_int32<18> (package_bits + i) << 6);
        }
        else
        {
            temp_data.last_data[counter] =
                (float)(cast_ganglion_bits_to_int32<19> (package_bits + i) << 5);
        }
    }

    // add first encoded package
    package[board_descr["default"]["package_num_channel"].get<int> ()] = data[0];
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
        eeg_scale * temp_data.last_data[0];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
        eeg_scale * temp_data.last_data[1];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
        eeg_scale * temp_data.last_data[2];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
        eeg_scale * temp_data.last_data[3];
    package[board_descr["default"]["accel_channels"][0].get<int> ()] = temp_data.accel_x;
    package[board_descr["default"]["accel_channels"][1].get<int> ()] = temp_data.accel_y;
    package[board_descr["default"]["accel_channels"][2].get<int> ()] = temp_data.accel_z;
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
    push_package (package);
    // add second package
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
        eeg_scale * temp_data.last_data[4];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
        eeg_scale * temp_data.last_data[5];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
        eeg_scale * temp_data.last_data[6];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
        eeg_scale * temp_data.last_data[7];
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
    push_package (package);
}

void GanglionNative::decompress_firmware_2 (const uint8_t *data, double *package)
{
    int bits_per_num = 0;
    unsigned char package_bits[160] = {0}; // 20 * 8
    for (int i = 0; i < 20; i++)
    {
        uchar_to_bits (data[i], package_bits + i * 8);
    }

    float delta[8] = {0.f}; // delta holds 8 nums (4 by each package)

    // no compression, used to init variable
    if (data[0] == 0)
    {
        // shift the last data packet to make room for a newer one
        temp_data.last_data[0] = temp_data.last_data[4];
        temp_data.last_data[1] = temp_data.last_data[5];
        temp_data.last_data[2] = temp_data.last_data[6];
        temp_data.last_data[3] = temp_data.last_data[7];

        // add new packet
        temp_data.last_data[4] = (float)cast_24bit_to_int32 (data + 1);
        temp_data.last_data[5] = (float)cast_24bit_to_int32 (data + 4);
        temp_data.last_data[6] = (float)cast_24bit_to_int32 (data + 7);
        temp_data.last_data[7] = (float)cast_24bit_to_int32 (data + 10);

        // scale new packet and insert into result
        package[board_descr["default"]["package_num_channel"].get<int> ()] = 0.;
        package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
            eeg_scale * temp_data.last_data[4];
        package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
            eeg_scale * temp_data.last_data[5];
        package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
            eeg_scale * temp_data.last_data[6];
        package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
            eeg_scale * temp_data.last_data[7];
        package[board_descr["default"]["accel_channels"][0].get<int> ()] = temp_data.accel_x;
        package[board_descr["default"]["accel_channels"][1].get<int> ()] = temp_data.accel_y;
        package[board_descr["default"]["accel_channels"][2].get<int> ()] = temp_data.accel_z;
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
        push_package (package);
        return;
    }
    // 18 bit compression, sends delta from previous value instead of real value!
    if (data[0] >= 1 && data[0] <= 100)
    {
        int last_digit = data[0] % 10;
        switch (last_digit)
        {
            // accel data is signed, so we must cast it to signed char
            // swap x and z, and invert z to convert to standard coordinate space.
            case 0:
                temp_data.accel_z = -accel_scale * (char)data[19];
                break;
            case 1:
                temp_data.accel_y = accel_scale * (char)data[19];
                break;
            case 2:
                temp_data.accel_x = accel_scale * (char)data[19];
                break;
            default:
                break;
        }
        bits_per_num = 18;
    }
    else if (data[0] >= 101 && data[0] <= 200)
    {
        bits_per_num = 19;
    }

    // handle compressed data for 18 or 19 bits
    for (int i = 8, counter = 0; i < bits_per_num * 8; i += bits_per_num, counter++)
    {
        if (bits_per_num == 18)
        {
            delta[counter] = (float)cast_ganglion_bits_to_int32<18> (package_bits + i);
        }
        else
        {
            delta[counter] = (float)cast_ganglion_bits_to_int32<19> (package_bits + i);
        }
    }

    // apply the first delta to the last data we got in the previous iteration
    for (int i = 0; i < 4; i++)
    {
        temp_data.last_data[i] = temp_data.last_data[i + 4] - delta[i];
    }

    // apply the second delta to the previous packet which we just decompressed above
    for (int i = 4; i < 8; i++)
    {
        temp_data.last_data[i] = temp_data.last_data[i - 4] - delta[i];
    }

    // add first encoded package
    package[board_descr["default"]["package_num_channel"].get<int> ()] = data[0];
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
        eeg_scale * temp_data.last_data[0];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
        eeg_scale * temp_data.last_data[1];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
        eeg_scale * temp_data.last_data[2];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
        eeg_scale * temp_data.last_data[3];
    package[board_descr["default"]["accel_channels"][0].get<int> ()] = temp_data.accel_x;
    package[board_descr["default"]["accel_channels"][1].get<int> ()] = temp_data.accel_y;
    package[board_descr["default"]["accel_channels"][2].get<int> ()] = temp_data.accel_z;
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
    push_package (package);
    // add second package
    package[board_descr["default"]["eeg_channels"][0].get<int> ()] =
        eeg_scale * temp_data.last_data[4];
    package[board_descr["default"]["eeg_channels"][1].get<int> ()] =
        eeg_scale * temp_data.last_data[5];
    package[board_descr["default"]["eeg_channels"][2].get<int> ()] =
        eeg_scale * temp_data.last_data[6];
    package[board_descr["default"]["eeg_channels"][3].get<int> ()] =
        eeg_scale * temp_data.last_data[7];
    package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();
    push_package (package);
}