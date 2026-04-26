#include "muse_anthena.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "custom_cast.h"
#include "muse_anthena_constants.h"
#include "timestamp.h"


MuseAnthena::SensorConfig::SensorConfig ()
    : type (SensorType::UNKNOWN)
    , n_channels (0)
    , n_samples (0)
    , sampling_rate (0.0)
    , data_len (0)
    , variable_length (false)
{
}

MuseAnthena::SensorConfig::SensorConfig (SensorType type, int n_channels, int n_samples,
    double sampling_rate, size_t data_len, bool variable_length)
    : type (type)
    , n_channels (n_channels)
    , n_samples (n_samples)
    , sampling_rate (sampling_rate)
    , data_len (data_len)
    , variable_length (variable_length)
{
}


bool MuseAnthena::get_sensor_config (uint8_t tag, SensorConfig &config)
{
    switch (tag)
    {
        case 0x11:
            config = SensorConfig (SensorType::EEG, 4, 4, 256.0, 28);
            return true;
        case 0x12:
            config = SensorConfig (SensorType::EEG, 8, 2, 256.0, 28);
            return true;
        case 0x34:
            config = SensorConfig (SensorType::OPTICS, 4, 3, 64.0, 30);
            return true;
        case 0x35:
            config = SensorConfig (SensorType::OPTICS, 8, 2, 64.0, 40);
            return true;
        case 0x36:
            config = SensorConfig (SensorType::OPTICS, 16, 1, 64.0, 40);
            return true;
        case 0x47:
            config = SensorConfig (SensorType::ACC_GYRO, 6, 3, 52.0, 36);
            return true;
        case 0x53:
            config = SensorConfig (SensorType::UNKNOWN, 0, 0, 0.0, 24);
            return true;
        case 0x88:
            config = SensorConfig (SensorType::BATTERY, 1, 1, 0.2, 0, true);
            return true;
        case 0x98:
            config = SensorConfig (SensorType::BATTERY, 1, 1, 1.0, 20);
            return true;
        default:
            return false;
    }
}

int MuseAnthena::get_optics_canonical_index (uint8_t tag, int channel)
{
    static const int optics4_indexes[] = {4, 5, 6, 7};

    if (tag == 0x34)
    {
        return (channel >= 0 && channel < 4) ? optics4_indexes[channel] : -1;
    }
    if ((tag == 0x35) && (channel >= 0) && (channel < 8))
    {
        return channel;
    }
    if ((tag == 0x36) && (channel >= 0) && (channel < 16))
    {
        return channel;
    }
    return -1;
}

double MuseAnthena::average_present (const std::array<double, 16> &values,
    const std::array<bool, 16> &present, const int *indexes, int len)
{
    double sum = 0.0;
    int count = 0;
    for (int i = 0; i < len; i++)
    {
        int index = indexes[i];
        if ((index >= 0) && (index < 16) && present[(size_t)index])
        {
            sum += values[(size_t)index];
            count++;
        }
    }
    return count > 0 ? sum / count : 0.0;
}


std::string MuseAnthena::trim_string (const std::string &value)
{
    size_t first = value.find_first_not_of (" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }
    size_t last = value.find_last_not_of (" \t\r\n");
    return value.substr (first, last - first + 1);
}

std::string MuseAnthena::to_lower (const std::string &value)
{
    std::string lower_value = value;
    std::transform (lower_value.begin (), lower_value.end (), lower_value.begin (),
        [] (unsigned char ch) { return (char)std::tolower (ch); });
    return lower_value;
}

bool MuseAnthena::is_valid_muse_preset (const std::string &preset)
{
    static const char *valid_presets[] = {"p20", "p21", "p50", "p51", "p60", "p61", "p1034",
        "p1035", "p1041", "p1042", "p1043", "p1044", "p1045", "p1046", "p4129"};

    for (size_t i = 0; i < sizeof (valid_presets) / sizeof (valid_presets[0]); i++)
    {
        if (preset == valid_presets[i])
        {
            return true;
        }
    }
    return false;
}

bool MuseAnthena::parse_bool_option (const std::string &value, bool &parsed)
{
    std::string lower_value = to_lower (trim_string (value));
    if (lower_value == "true")
    {
        parsed = true;
        return true;
    }
    if (lower_value == "false")
    {
        parsed = false;
        return true;
    }
    return false;
}

int MuseAnthena::parse_muse_options ()
{
    muse_preset = "p1041";
    enable_low_latency = true;

    std::string other_info = trim_string (params.other_info);
    if (other_info.empty ())
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    if ((other_info.find ('=') == std::string::npos) &&
        (other_info.find (';') == std::string::npos))
    {
        std::string preset = to_lower (other_info);
        if (!is_valid_muse_preset (preset))
        {
            safe_logger (spdlog::level::err, "Invalid MuseAnthena preset in other_info: {}",
                other_info);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        muse_preset = preset;
        return (int)BrainFlowExitCodes::STATUS_OK;
    }

    bool has_options = false;
    std::stringstream ss (other_info);
    std::string token;
    while (std::getline (ss, token, ';'))
    {
        token = trim_string (token);
        if (token.empty ())
        {
            continue;
        }

        size_t separator = token.find ('=');
        if ((separator == std::string::npos) ||
            (token.find ('=', separator + 1) != std::string::npos))
        {
            safe_logger (spdlog::level::err,
                "Invalid MuseAnthena other_info option: {}. Expected key=value", token);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }

        std::string key = to_lower (trim_string (token.substr (0, separator)));
        std::string value = trim_string (token.substr (separator + 1));
        if (key.empty () || value.empty ())
        {
            safe_logger (spdlog::level::err,
                "Invalid MuseAnthena other_info option: {}. Empty key or value", token);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }

        if (key == "preset")
        {
            value = to_lower (value);
            if (!is_valid_muse_preset (value))
            {
                safe_logger (
                    spdlog::level::err, "Invalid MuseAnthena preset in other_info: {}", value);
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }
            muse_preset = value;
            has_options = true;
        }
        else if (key == "low_latency")
        {
            bool parsed = false;
            if (!parse_bool_option (value, parsed))
            {
                safe_logger (spdlog::level::err,
                    "Invalid MuseAnthena low_latency value in other_info: {}", value);
                return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
            }
            enable_low_latency = parsed;
            has_options = true;
        }
        else
        {
            safe_logger (spdlog::level::err, "Unknown MuseAnthena other_info option: {}", key);
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    if (!has_options)
    {
        safe_logger (spdlog::level::err, "Invalid MuseAnthena other_info: {}", params.other_info);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}


void anthena_adapter_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((MuseAnthena *)(board))->adapter_on_scan_found (adapter, peripheral);
}

void anthena_peripheral_on_data (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((MuseAnthena *)(board))->peripheral_on_data (peripheral, service, characteristic, data, size);
}

void anthena_peripheral_on_status (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((MuseAnthena *)(board))
        ->peripheral_on_status (peripheral, service, characteristic, data, size);
}

MuseAnthena::MuseAnthena (int board_id, struct BrainFlowInputParams params)
    : BLELibBoard (board_id, params)
{
    initialized = false;
    muse_adapter = NULL;
    muse_peripheral = NULL;
    is_streaming = false;
    timestamp_initialized = false;
    first_device_tick = 0;
    first_host_timestamp = 0.0;
    last_battery = 0.0;
    muse_preset = "p1041";
    enable_low_latency = true;
}

MuseAnthena::~MuseAnthena ()
{
    skip_logs = true;
    release_session ();
}

int MuseAnthena::prepare_session ()
{
    if (initialized)
    {
        safe_logger (spdlog::level::info, "Session is already prepared");
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    int res = parse_muse_options ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    safe_logger (spdlog::level::info, "Use MuseAnthena preset {} and low_latency {}",
        muse_preset, enable_low_latency);
    if (params.timeout < 1)
    {
        params.timeout = 6;
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

    safe_logger (spdlog::level::info, "found {} BLE adapter(s)", adapter_count);

    muse_adapter = simpleble_adapter_get_handle (0);
    if (muse_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    simpleble_adapter_set_callback_on_scan_found (
        muse_adapter, ::anthena_adapter_on_scan_found, (void *)this);

    if (!simpleble_adapter_is_bluetooth_enabled ())
    {
        safe_logger (spdlog::level::warn, "Probably bluetooth is disabled.");
        // dont throw an exception because of this
        // https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/115
    }

    simpleble_adapter_scan_start (muse_adapter);
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, params.timeout * sec, [this] { return this->muse_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found MuseAnthena device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to find MuseAnthena Device");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (muse_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // for safety
        for (int i = 0; i < 3; i++)
        {
            if (simpleble_peripheral_connect (muse_peripheral) == SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::info, "Connected to MuseAnthena Device");
                res = (int)BrainFlowExitCodes::STATUS_OK;
                break;
            }
            else
            {
                safe_logger (
                    spdlog::level::warn, "Failed to connect to MuseAnthena Device: {}/3", i);
                res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
                std::this_thread::sleep_for (std::chrono::seconds (1));
            }
        }
    }

    // https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/26#issuecomment-955606799
    std::this_thread::sleep_for (std::chrono::seconds (1));

    bool control_characteristics_found = false;

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        size_t services_count = simpleble_peripheral_services_count (muse_peripheral);
        for (size_t i = 0; i < services_count; i++)
        {
            simpleble_service_t service;
            if (simpleble_peripheral_services_get (muse_peripheral, i, &service) !=
                SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::err, "failed to get service");
                res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
            }

            safe_logger (spdlog::level::trace, "found service {}", service.uuid.value);
            for (size_t j = 0; j < service.characteristic_count; j++)
            {
                safe_logger (spdlog::level::trace, "found characteristic {}",
                    service.characteristics[j].uuid.value);

                if (strcmp (service.characteristics[j].uuid.value,
                        MUSE_ANTHENA_GATT_ATTR_STREAM_TOGGLE) == 0)
                {
                    control_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j].uuid);
                    control_characteristics_found = true;
                    safe_logger (spdlog::level::info, "found control characteristic");
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::anthena_peripheral_on_status,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j].uuid));
                    }
                    else
                    {
                        safe_logger (spdlog::level::warn, "Failed to notify for control {} {}",
                            service.uuid.value, service.characteristics[j].uuid.value);
                    }
                }

                if ((strcmp (service.characteristics[j].uuid.value, MUSE_ANTHENA_GATT_DATA_1) == 0) ||
                    (strcmp (service.characteristics[j].uuid.value, MUSE_ANTHENA_GATT_DATA_2) == 0))
                {
                    // Athena multiplexes EEG, IMU, optics, and battery packets across data
                    // characteristics; use one parser intentionally and route by packet tag.
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::anthena_peripheral_on_data,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j].uuid));
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

    if ((res == (int)BrainFlowExitCodes::STATUS_OK) && (!control_characteristics_found))
    {
        safe_logger (spdlog::level::err, "failed to find control characteristic");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        initialized = true;
        res = config_board ("v6");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
        res = config_board ("s");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
        res = config_board ("h");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
        res = config_board (muse_preset);
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
        res = config_board ("s");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
    }
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        release_session ();
    }

    return res;
}

int MuseAnthena::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::lock_guard<std::mutex> callback_guard (callback_lock);
        timestamp_initialized = false;
        first_device_tick = 0;
        first_host_timestamp = 0.0;
        last_battery = 0.0;
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("dc001");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
        res = config_board ("dc001");
    }
    if ((res == (int)BrainFlowExitCodes::STATUS_OK) && enable_low_latency)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        int l1_res = config_board ("L1");
        if (l1_res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::warn, "Failed to enable MuseAnthena low latency mode");
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (300));
        int status_res = config_board ("s");
        if (status_res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            safe_logger (spdlog::level::warn, "Failed to request MuseAnthena status after start");
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        is_streaming = true;
    }

    return res;
}

int MuseAnthena::stop_stream ()
{
    if (muse_peripheral == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (is_streaming)
    {
        res = config_board ("h");
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            bool is_connected = false;
            if ((simpleble_peripheral_is_connected (muse_peripheral, &is_connected) ==
                    SIMPLEBLE_SUCCESS) &&
                (!is_connected))
            {
                safe_logger (spdlog::level::warn,
                    "MuseAnthena device is already disconnected during stop_stream");
                res = (int)BrainFlowExitCodes::STATUS_OK;
            }
        }
    }
    else
    {
        res = (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    is_streaming = false;
    timestamp_initialized = false;
    return res;
}

int MuseAnthena::release_session ()
{
    if (initialized)
    {
        // repeat it multiple times, failure here may lead to a crash
        for (int i = 0; i < 2; i++)
        {
            bool is_connected = true;
            if ((muse_peripheral != NULL) &&
                (simpleble_peripheral_is_connected (muse_peripheral, &is_connected) ==
                    SIMPLEBLE_SUCCESS) &&
                (!is_connected))
            {
                break;
            }

            stop_stream ();
            if (muse_peripheral != NULL)
            {
                // need to wait for notifications to stop triggered before unsubscribing, otherwise
                // macos fails inside simpleble with timeout
                std::this_thread::sleep_for (std::chrono::seconds (2));
                for (auto notified : notified_characteristics)
                {
                    if (simpleble_peripheral_unsubscribe (
                            muse_peripheral, notified.first, notified.second) != SIMPLEBLE_SUCCESS)
                    {
                        safe_logger (spdlog::level::err, "failed to unsubscribe for {} {}",
                            notified.first.value, notified.second.value);
                    }
                }
            }
        }
        free_packages ();
        initialized = false;
        notified_characteristics.clear ();
    }
    if (muse_peripheral != NULL)
    {
        bool is_connected = false;
        if (simpleble_peripheral_is_connected (muse_peripheral, &is_connected) == SIMPLEBLE_SUCCESS)
        {
            if (is_connected)
            {
                simpleble_peripheral_disconnect (muse_peripheral);
            }
        }
        simpleble_peripheral_release_handle (muse_peripheral);
        muse_peripheral = NULL;
    }
    if (muse_adapter != NULL)
    {
        simpleble_adapter_release_handle (muse_adapter);
        muse_adapter = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseAnthena::config_board (std::string config, std::string &response)
{
    return config_board (config);
}

int MuseAnthena::config_board (std::string config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    constexpr int max_size = 16;
    uint8_t command[max_size];
    size_t len = config.size ();
    if (len + 2 >= max_size)
    {
        safe_logger (spdlog::level::err, "Invalid command, max size is {}", max_size);
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    command[0] = (uint8_t)len + 1;
    for (size_t i = 0; i < len; i++)
    {
        command[i + 1] = uint8_t (config[i]);
    }
    command[len + 1] = 10;
    safe_logger (spdlog::level::trace, "Command to send: {}", bytes_to_string (command, len + 2));
    if (simpleble_peripheral_write_command (muse_peripheral, control_characteristics.first,
            control_characteristics.second, command, len + 2) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void MuseAnthena::adapter_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral)
{
    (void)adapter;
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
            if (strncmp (peripheral_identified, "MuseS", 5) == 0)
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
            muse_peripheral = peripheral;
        }
        cv.notify_one ();
    }
    else
    {
        simpleble_peripheral_release_handle (peripheral);
    }
}

void MuseAnthena::peripheral_on_data (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    (void)peripheral;
    (void)service;
    (void)characteristic;
    handle_data_notification (data, size);
}

void MuseAnthena::peripheral_on_status (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    (void)peripheral;
    (void)service;
    (void)characteristic;
    safe_logger (spdlog::level::debug, "Status packet: {}", bytes_to_string (data, size));
}

void MuseAnthena::handle_data_notification (const uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> callback_guard (callback_lock);

    size_t offset = 0;
    while (offset < size)
    {
        if (size - offset < PACKET_HEADER_SIZE)
        {
            safe_logger (spdlog::level::warn, "Short Athena message tail: {} bytes", size - offset);
            return;
        }

        uint8_t packet_len = data[offset];
        if ((packet_len < PACKET_HEADER_SIZE) || (offset + packet_len > size))
        {
            safe_logger (spdlog::level::warn, "Invalid Athena packet length: {}", packet_len);
            return;
        }

        const uint8_t *packet = data + offset;
        uint8_t packet_index = packet[1];
        uint32_t device_tick =
            cast_32bit_to_uint32_little_endian ((const unsigned char *)(packet + 2));
        uint8_t primary_tag = packet[9];
        const uint8_t *packet_data = packet + PACKET_HEADER_SIZE;
        size_t packet_data_size = packet_len - PACKET_HEADER_SIZE;
        size_t packet_data_offset = 0;

        SensorConfig primary_config;
        if (get_sensor_config (primary_tag, primary_config))
        {
            size_t primary_data_len =
                primary_config.variable_length ? packet_data_size : primary_config.data_len;
            if ((primary_data_len > 0) && (primary_data_len <= packet_data_size))
            {
                parse_sensor_payload (
                    primary_tag, packet_index, device_tick, packet_data, primary_data_len);
                packet_data_offset = primary_data_len;
            }
            else
            {
                safe_logger (spdlog::level::warn,
                    "Invalid Athena primary payload for tag 0x{:02x}: {} bytes",
                    (unsigned int)primary_tag, packet_data_size);
                packet_data_offset = packet_data_size;
            }
        }
        else
        {
            safe_logger (spdlog::level::trace, "Unknown Athena primary tag: 0x{:02x}",
                (unsigned int)primary_tag);
            packet_data_offset = packet_data_size;
        }

        while (packet_data_offset + SUBPACKET_HEADER_SIZE <= packet_data_size)
        {
            uint8_t tag = packet_data[packet_data_offset];
            uint8_t subpacket_index = packet_data[packet_data_offset + 1];
            SensorConfig config;
            if (!get_sensor_config (tag, config))
            {
                safe_logger (spdlog::level::trace, "Unknown Athena subpacket tag: 0x{:02x}",
                    (unsigned int)tag);
                break;
            }

            size_t remaining = packet_data_size - packet_data_offset - SUBPACKET_HEADER_SIZE;
            size_t sensor_data_len = config.variable_length ? remaining : config.data_len;
            if ((sensor_data_len == 0) || (sensor_data_len > remaining))
            {
                safe_logger (spdlog::level::warn,
                    "Invalid Athena subpacket payload for tag 0x{:02x}: {} bytes",
                    (unsigned int)tag, remaining);
                break;
            }

            parse_sensor_payload (tag, subpacket_index, device_tick,
                packet_data + packet_data_offset + SUBPACKET_HEADER_SIZE, sensor_data_len);
            packet_data_offset += SUBPACKET_HEADER_SIZE + sensor_data_len;
        }

        offset += packet_len;
    }
}

void MuseAnthena::parse_sensor_payload (
    uint8_t tag, uint8_t sequence_num, uint32_t device_tick, const uint8_t *data, size_t size)
{
    SensorConfig config;
    if (!get_sensor_config (tag, config))
    {
        return;
    }

    if (config.type == SensorType::UNKNOWN)
    {
        safe_logger (spdlog::level::trace, "Skipping unknown Athena payload tag 0x{:02x}",
            (unsigned int)tag);
        return;
    }

    if (config.type == SensorType::BATTERY)
    {
        if (size >= 2)
        {
            last_battery =
                (double)cast_16bit_to_uint16_little_endian ((const unsigned char *)data) / 256.0;
        }
        return;
    }

    if ((!config.variable_length) && (size < config.data_len))
    {
        safe_logger (spdlog::level::warn, "Short Athena payload for tag 0x{:02x}: {}",
            (unsigned int)tag, size);
        return;
    }

    if (config.type == SensorType::EEG)
    {
        int num_rows = board_descr["default"]["num_rows"].get<int> ();
        std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
        int package_num_channel = board_descr["default"]["package_num_channel"].get<int> ();
        int timestamp_channel = board_descr["default"]["timestamp_channel"].get<int> ();

        for (int sample = 0; sample < config.n_samples; sample++)
        {
            std::vector<double> package ((size_t)num_rows, 0.0);
            package[(size_t)package_num_channel] = (double)sequence_num;
            for (int channel = 0; channel < config.n_channels; channel++)
            {
                size_t bit_start = (size_t)(sample * config.n_channels + channel) * 14;
                uint32_t raw = extract_lsb_bits ((const unsigned char *)data, bit_start, 14);
                if ((size_t)channel < eeg_channels.size ())
                {
                    package[(size_t)eeg_channels[(size_t)channel]] =
                        (double)raw * MUSE_ANTHENA_EEG_SCALE_FACTOR;
                }
            }
            package[(size_t)timestamp_channel] =
                get_sample_timestamp (device_tick, sample, config.sampling_rate);
            push_package (package.data (), (int)BrainFlowPresets::DEFAULT_PRESET);
        }
        return;
    }

    if (config.type == SensorType::ACC_GYRO)
    {
        int num_rows = board_descr["auxiliary"]["num_rows"].get<int> ();
        std::vector<int> accel_channels = board_descr["auxiliary"]["accel_channels"];
        std::vector<int> gyro_channels = board_descr["auxiliary"]["gyro_channels"];
        int package_num_channel = board_descr["auxiliary"]["package_num_channel"].get<int> ();
        int timestamp_channel = board_descr["auxiliary"]["timestamp_channel"].get<int> ();

        for (int sample = 0; sample < config.n_samples; sample++)
        {
            std::vector<double> package ((size_t)num_rows, 0.0);
            package[(size_t)package_num_channel] = (double)sequence_num;
            for (int channel = 0; channel < 3; channel++)
            {
                int16_t raw = cast_16bit_to_int16_little_endian (
                    (const unsigned char *)(data + (sample * config.n_channels + channel) * 2));
                if ((size_t)channel < accel_channels.size ())
                {
                    package[(size_t)accel_channels[(size_t)channel]] =
                        (double)raw * MUSE_ANTHENA_ACCELEROMETER_SCALE_FACTOR;
                }
            }
            for (int channel = 0; channel < 3; channel++)
            {
                int16_t raw = cast_16bit_to_int16_little_endian ((const unsigned char *)(data +
                    (sample * config.n_channels + channel + 3) * 2));
                if ((size_t)channel < gyro_channels.size ())
                {
                    package[(size_t)gyro_channels[(size_t)channel]] =
                        (double)raw * MUSE_ANTHENA_GYRO_SCALE_FACTOR;
                }
            }
            package[(size_t)timestamp_channel] =
                get_sample_timestamp (device_tick, sample, config.sampling_rate);
            push_package (package.data (), (int)BrainFlowPresets::AUXILIARY_PRESET);
        }
        return;
    }

    if (config.type == SensorType::OPTICS)
    {
        static const int nir_indexes[] = {0, 1, 4, 5};
        static const int red_indexes[] = {8, 9, 12, 13};
        static const int ir_indexes[] = {2, 3, 6, 7};

        int num_rows = board_descr["ancillary"]["num_rows"].get<int> ();
        std::vector<int> ppg_channels = board_descr["ancillary"]["ppg_channels"];
        std::vector<int> optical_channels = board_descr["ancillary"]["optical_channels"];
        int package_num_channel = board_descr["ancillary"]["package_num_channel"].get<int> ();
        int timestamp_channel = board_descr["ancillary"]["timestamp_channel"].get<int> ();
        int battery_channel = board_descr["ancillary"]["battery_channel"].get<int> ();

        for (int sample = 0; sample < config.n_samples; sample++)
        {
            std::array<double, 16> optical_values;
            std::array<bool, 16> optical_present;
            optical_values.fill (0.0);
            optical_present.fill (false);

            std::vector<double> package ((size_t)num_rows, 0.0);
            package[(size_t)package_num_channel] = (double)sequence_num;
            package[(size_t)battery_channel] = last_battery;

            for (int channel = 0; channel < config.n_channels; channel++)
            {
                size_t bit_start = (size_t)(sample * config.n_channels + channel) * 20;
                uint32_t raw = extract_lsb_bits ((const unsigned char *)data, bit_start, 20);
                int canonical_index = get_optics_canonical_index (tag, channel);
                if ((canonical_index >= 0) && (canonical_index < 16))
                {
                    double value = (double)raw * MUSE_ANTHENA_OPTICS_SCALE_FACTOR;
                    optical_values[(size_t)canonical_index] = value;
                    optical_present[(size_t)canonical_index] = true;
                    if ((size_t)canonical_index < optical_channels.size ())
                    {
                        package[(size_t)optical_channels[(size_t)canonical_index]] = value;
                    }
                }
            }

            if (ppg_channels.size () >= 3)
            {
                package[(size_t)ppg_channels[0]] =
                    average_present (optical_values, optical_present, nir_indexes, 4);
                package[(size_t)ppg_channels[1]] =
                    average_present (optical_values, optical_present, red_indexes, 4);
                package[(size_t)ppg_channels[2]] =
                    average_present (optical_values, optical_present, ir_indexes, 4);
            }
            package[(size_t)timestamp_channel] =
                get_sample_timestamp (device_tick, sample, config.sampling_rate);
            push_package (package.data (), (int)BrainFlowPresets::ANCILLARY_PRESET);
        }
    }
}

double MuseAnthena::get_sample_timestamp (
    uint32_t device_tick, int sample_index, double sampling_rate)
{
    if (!timestamp_initialized)
    {
        timestamp_initialized = true;
        first_device_tick = device_tick;
        first_host_timestamp = get_timestamp ();
    }

    uint32_t elapsed_ticks = device_tick - first_device_tick;
    return first_host_timestamp + (double)elapsed_ticks / MUSE_ANTHENA_DEVICE_CLOCK_HZ +
        (double)sample_index / sampling_rate;
}

std::string MuseAnthena::bytes_to_string (const uint8_t *data, size_t size)
{
    std::ostringstream oss;
    for (size_t i = 0; i < size; i++)
    {
        if (i > 0)
        {
            oss << " ";
        }
        oss << static_cast<int> (data[i]);
    }
    return oss.str ();
}
