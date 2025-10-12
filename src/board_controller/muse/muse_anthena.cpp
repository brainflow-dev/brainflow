#include "muse_anthena.h"

#include <cstdint>
#include <sstream>
#include <string>

#include "custom_cast.h"
#include "muse_anthena_constants.h"
#include "timestamp.h"


void anthena_adapter_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((MuseAnthena *)(board))->adapter_on_scan_found (adapter, peripheral);
}

void anthena_peripheral_on_eeg (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((MuseAnthena *)(board))->peripheral_on_eeg (peripheral, service, characteristic, data, size);
}

void anthena_peripheral_on_aux (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((MuseAnthena *)(board))->peripheral_on_aux (peripheral, service, characteristic, data, size);
}

MuseAnthena::MuseAnthena (int board_id, struct BrainFlowInputParams params)
    : BLELibBoard (board_id, params)
{
    initialized = false;
    muse_adapter = NULL;
    muse_peripheral = NULL;
    is_streaming = false;
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
    int res = (int)BrainFlowExitCodes::STATUS_OK;
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
#ifdef _WIN32
                Sleep (1000);
#else
                sleep (1);
#endif
            }
        }
    }

// https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/26#issuecomment-955606799
#ifdef _WIN32
    Sleep (1000);
#else
    sleep (1);
#endif

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

            safe_logger (spdlog::level::trace, "found servce {}", service.uuid.value);
            for (size_t j = 0; j < service.characteristic_count; j++)
            {
                safe_logger (spdlog::level::trace, "found characteristic {}",
                    service.characteristics[j].uuid.value);

                // control characteristic
                if (strcmp (service.characteristics[j].uuid.value,
                        MUSE_ANTHENA_GATT_ATTR_STREAM_TOGGLE) == 0)
                {
                    control_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j].uuid);
                    control_characteristics_found = true;
                    safe_logger (spdlog::level::info, "found control characteristic");
                }

                // eeg characteristic
                if (strcmp (service.characteristics[j].uuid.value, MUSE_ANTHENA_GATT_EEG) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::anthena_peripheral_on_eeg,
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

                // aux characteristic(acce, gyro, etc)
                if (strcmp (service.characteristics[j].uuid.value, MUSE_ANTHENA_GATT_AUX) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::anthena_peripheral_on_aux,
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

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        initialized = true;
        // used to be v1 for old muse devices
        res = config_board ("v4");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // p21 - eeg only
        res = config_board ("p21");
    }
    else
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
        res = config_board ("dc001");
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
    }
    else
    {
        res = (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    is_streaming = false;
    return res;
}

int MuseAnthena::release_session ()
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
        free_packages ();
        initialized = false;
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
            if (strncmp (peripheral_identified, "MuseS", 4) == 0)
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

void MuseAnthena::peripheral_on_eeg (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "EEG packet: {}", bytes_to_string (data, size));
    // TODO parse it and fill default preset
}

void MuseAnthena::peripheral_on_aux (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "AUX packet: {}", bytes_to_string (data, size));
    // TODO parse it and fill aux preset
}


std::string MuseAnthena::bytes_to_string (const uint8_t *data, size_t size)
{
    std::ostringstream oss;
    for (size_t i = 0; i < size; i++)
    {
        if (i > 0)
            oss << " ";
        oss << static_cast<int> (data[i]);
    }
    return oss.str ();
}