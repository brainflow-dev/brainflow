#include <string.h>

#include "synchroni_board.h"

#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"

#define SYNCHRONI_WRITE_CHAR "f000ffe1-0451-4000-b000-000000000000"
#define SYNCHRONI_NOTIFY_CHAR "f000ffe2-0451-4000-b000-000000000000"


static void synchroni_board_adapter_1_on_scan_updated (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((SynchroniBoard *)(board))->adapter_1_on_scan_updated (adapter, peripheral);
}

static void synchroni_board_read_notifications (simpleble_uuid_t service,
    simpleble_uuid_t characteristic, uint8_t *data, size_t size, void *board)
{
    ((SynchroniBoard *)(board))->read_data (service, characteristic, data, size);
}

SynchroniBoard::SynchroniBoard (int board_id, struct BrainFlowInputParams params)
    : BLELibBoard (board_id, params)
{
    initialized = false;
    synchroni_board_adapter = NULL;
    synchroni_board_peripheral = NULL;
    is_streaming = false;
}

SynchroniBoard::~SynchroniBoard ()
{
    skip_logs = true;
    release_session ();
}

int SynchroniBoard::prepare_session ()
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

    synchroni_board_adapter = simpleble_adapter_get_handle (0);
    if (synchroni_board_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    simpleble_adapter_set_callback_on_scan_updated (
        synchroni_board_adapter, ::synchroni_board_adapter_1_on_scan_updated, (void *)this);

    if (!simpleble_adapter_is_bluetooth_enabled ())
    {
        safe_logger (spdlog::level::warn, "Probably bluetooth is disabled.");
        // dont throw an exception because of this
        // https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/115
    }

    simpleble_adapter_scan_start (synchroni_board_adapter);
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (
            lk, params.timeout * sec, [this] { return this->synchroni_board_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found SynchroniBoard device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to find SynchroniBoard Device");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (synchroni_board_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // for safety
        for (int i = 0; i < 3; i++)
        {
            if (simpleble_peripheral_connect (synchroni_board_peripheral) == SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::info, "Connected to SynchroniBoard Device");
                res = (int)BrainFlowExitCodes::STATUS_OK;

                break;
            }
            else
            {
                safe_logger (
                    spdlog::level::warn, "Failed to connect to SynchroniBoard Device: {}/3", i);
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
        size_t services_count = simpleble_peripheral_services_count (synchroni_board_peripheral);
        for (size_t i = 0; i < services_count; i++)
        {
            simpleble_service_t service;
            if (simpleble_peripheral_services_get (synchroni_board_peripheral, i, &service) !=
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
                        SYNCHRONI_WRITE_CHAR) == 0) // Write Characteristics
                {
                    write_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j].uuid);
                    control_characteristics_found = true;
                    safe_logger (spdlog::level::info, "found control characteristic");
                }
                if (strcmp (service.characteristics[j].uuid.value,
                        SYNCHRONI_NOTIFY_CHAR) == 0) // Notification Characteristics
                {
                    if (simpleble_peripheral_notify (synchroni_board_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::synchroni_board_read_notifications,
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

int SynchroniBoard::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = prepare_for_acquisition (buffer_size, streamer_params);

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // todo send smth to enable streaming
        res = config_board ("0x0FFFFFFFF");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        is_streaming = true;
    }
    safe_logger (spdlog::level::trace, "Stream Status is {}",
                    is_streaming);
    return res;
}

int SynchroniBoard::stop_stream ()
{
    if (synchroni_board_peripheral == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (is_streaming)
    {
        res = config_board ("0x4000000d");
    }
    else
    {
        res = (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    is_streaming = false;
    safe_logger (spdlog::level::trace, "Stream Status is {} res is {}, {}",
                    is_streamingm,res,(int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR);
    return res;
}

int SynchroniBoard::release_session ()
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
            if (simpleble_peripheral_unsubscribe (synchroni_board_peripheral,
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
    if (synchroni_board_peripheral != NULL)
    {
        bool is_connected = false;
        if (simpleble_peripheral_is_connected (synchroni_board_peripheral, &is_connected) ==
            SIMPLEBLE_SUCCESS)
        {
            if (is_connected)
            {
                simpleble_peripheral_disconnect (synchroni_board_peripheral);
            }
        }
        simpleble_peripheral_release_handle (synchroni_board_peripheral);
        synchroni_board_peripheral = NULL;
    }
    if (synchroni_board_adapter != NULL)
    {
        simpleble_adapter_release_handle (synchroni_board_adapter);
        synchroni_board_adapter = NULL;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int SynchroniBoard::config_board (std::string config, std::string &response)
{
    return config_board (config);
}

int SynchroniBoard::config_board (std::string config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

    uint8_t *command = new uint8_t[config.size ()];
    memcpy (command, config.c_str (), config.size ());
    if (simpleble_peripheral_write_command (synchroni_board_peripheral, write_characteristics.first,
            write_characteristics.second, command, config.size ()) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        delete[] command;
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    delete[] command;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void SynchroniBoard::adapter_1_on_scan_updated (
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
            std::string name = board_descr["default"]["name"];
            if (strncmp (peripheral_identified, name.c_str (), name.size ()) == 0)
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
            synchroni_board_peripheral = peripheral;
        }
        cv.notify_one ();
    }
    else
    {
        simpleble_peripheral_release_handle (peripheral);
    }
}

void SynchroniBoard::read_data (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    // todo implement
    for (size_t i = 0; i < size; i++)
    {
        safe_logger (spdlog::level::info, "data: {} {}", i, data[i]);
    }
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];
    for (int i = 0; i < num_rows; i++)
        {
            package[i] = 0.0;
        }

    for (int i = 0; i < 20; i++)
    {
        uchar_to_bits (data[i], package_bits + i * 8);
    }
    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    std::vector<int> accel_channels = board_descr["default"]["accel_channels"];
    std::vector<int> gyro_channels = board_descr["default"]["gyro_channels"];
    if (data[1] >= 160 && data[1] <= 170){
        switch(data[1])
        {
            case 160:
            uint16_t sampleRate = static_cast<uint16_t>(data[2]) | (static_cast<uint16_t>(data[3]) << 8);
            uint64_t conversionFactorBits = 0;
            for (int i = 0; i < 8; ++i) {
                conversionFactorBits |= (static_cast<uint64_t>(data[14 + i]) << (8 * i));
            }
        }

    }
}
