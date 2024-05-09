#include <string>

#include "brainalive.h"
#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"

// common constants
#define BRAINALIVE_PACKET_SIZE 224

// info about services and chars
#define START_BYTE 0x0A
#define STOP_BYTE 0x0D

#define BRAINALIVE_WRITE_CHAR "0000fe41-8e22-4541-9d4c-21edae82ed19"
#define BRAINALIVE_NOTIFY_CHAR "0000fe42-8e22-4541-9d4c-21edae82ed19"

// info for equations
#define BRAINALIVE_EEG_SCALE_FACTOR 0.0476837158203125
#define BRAINALIVE_EEG_GAIN_VALUE 12


static void brainalive_adapter_1_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((BrainAlive *)(board))->adapter_1_on_scan_found (adapter, peripheral);
}

static void brainalive_read_notifications (simpleble_uuid_t service,
    simpleble_uuid_t characteristic, uint8_t *data, size_t size, void *board)
{
    ((BrainAlive *)(board))->read_data (service, characteristic, data, size, 0);
}

BrainAlive::BrainAlive (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::BRAINALIVE_BOARD, params)
{
    initialized = false;
    brainalive_adapter = NULL;
    brainalive_peripheral = NULL;
    is_streaming = false;
}

BrainAlive::~BrainAlive ()
{
    skip_logs = true;
    release_session ();
}

int BrainAlive::prepare_session ()
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

    brainalive_adapter = simpleble_adapter_get_handle (0);
    if (brainalive_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    simpleble_adapter_set_callback_on_scan_found (
        brainalive_adapter, ::brainalive_adapter_1_on_scan_found, (void *)this);

    if (!simpleble_adapter_is_bluetooth_enabled ())
    {
        safe_logger (spdlog::level::warn, "Probably bluetooth is disabled.");
        // dont throw an exception because of this
        // https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/115
    }

    simpleble_adapter_scan_start (brainalive_adapter);
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (
            lk, params.timeout * sec, [this] { return this->brainalive_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found BrainAlive device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to find BrainAlive Device");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (brainalive_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // for safety
        for (int i = 0; i < 3; i++)
        {
            if (simpleble_peripheral_connect (brainalive_peripheral) == SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::info, "Connected to BrainAlive Device");
                res = (int)BrainFlowExitCodes::STATUS_OK;
                break;
            }
            else
            {
                safe_logger (
                    spdlog::level::warn, "Failed to connect to BrainAlive Device: {}/3", i);
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

    bool control_characteristics_found = false;

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        size_t services_count = simpleble_peripheral_services_count (brainalive_peripheral);
        for (size_t i = 0; i < services_count; i++)
        {
            simpleble_service_t service;
            if (simpleble_peripheral_services_get (brainalive_peripheral, i, &service) !=
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
                        BRAINALIVE_WRITE_CHAR) == 0) // Write Characteristics
                {
                    write_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j].uuid);
                    control_characteristics_found = true;
                    safe_logger (spdlog::level::info, "found control characteristic");
                }
                if (strcmp (service.characteristics[j].uuid.value,
                        BRAINALIVE_NOTIFY_CHAR) == 0) // Notification Characteristics
                {
                    if (simpleble_peripheral_notify (brainalive_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::brainalive_read_notifications,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {

                        notified_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                            service.uuid, service.characteristics[j].uuid);
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

    if ((res == (int)BrainFlowExitCodes::STATUS_OK) && (control_characteristics_found))
    {
        initialized = true;
    }
    else
    {
        release_session ();
    }
    return res;
}

int BrainAlive::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("0a8100000d");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        safe_logger (spdlog::level::debug, "Start command Send 250sps");
        is_streaming = true;
    }

    return res;
}

int BrainAlive::stop_stream ()
{
    if (brainalive_peripheral == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (is_streaming)
    {
        res = config_board ("0a4000000d");
    }
    else
    {
        res = (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    is_streaming = false;
    return res;
}

int BrainAlive::release_session ()
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
            if (simpleble_peripheral_unsubscribe (brainalive_peripheral,
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
    if (brainalive_peripheral != NULL)
    {
        bool is_connected = false;
        if (simpleble_peripheral_is_connected (brainalive_peripheral, &is_connected) ==
            SIMPLEBLE_SUCCESS)
        {
            if (is_connected)
            {
                simpleble_peripheral_disconnect (brainalive_peripheral);
            }
        }
        simpleble_peripheral_release_handle (brainalive_peripheral);
        brainalive_peripheral = NULL;
    }
    if (brainalive_adapter != NULL)
    {
        simpleble_adapter_release_handle (brainalive_adapter);
        brainalive_adapter = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainAlive::config_board (std::string config, std::string &response)
{
    return config_board (config);
}

int BrainAlive::config_board (std::string config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    uint8_t command[5];
    size_t len = config.size ();
    command[0] = 0x0a;
    command[1] = 0x81; // it is hardcoded for now only
    command[2] = 0x00;
    command[3] = 0x00;
    command[4] = 0x0d;
    safe_logger (spdlog::level::trace, config[2]);
    if (simpleble_peripheral_write_command (brainalive_peripheral, write_characteristics.first,
            write_characteristics.second, command, sizeof (command)) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void BrainAlive::adapter_1_on_scan_found (
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
            if (strncmp (peripheral_identified, "ORION_1", 7) == 0)
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
            brainalive_peripheral = peripheral;
        }
        cv.notify_one ();
    }
    else
    {
        simpleble_peripheral_release_handle (peripheral);
    }
}

void BrainAlive::read_data (simpleble_uuid_t service, simpleble_uuid_t characteristic,
    uint8_t *data, size_t size, int channel_num)
{
    if (size != BRAINALIVE_PACKET_SIZE)
    {
        safe_logger (spdlog::level::warn, "unknown size of BrainAlive Data {}", size);
        return;
    }

    for (int i = 0; i < (int)size; i += 32)
    {
        double eeg_data[9] = {0};
        for (int j = i + 4, k = 0; j < i + 28; j += 3, k++)
        {
            eeg_data[k] = (((data[j] << 16 | data[j + 1] << 8 | data[j + 2]) << 8) >> 8) *
                BRAINALIVE_EEG_SCALE_FACTOR / BRAINALIVE_EEG_GAIN_VALUE;
        }
        eeg_data[8] = data[i + 29];
        push_package (&eeg_data[0]);
    }
}
