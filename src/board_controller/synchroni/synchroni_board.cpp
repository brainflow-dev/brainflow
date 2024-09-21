#include "synchroni_board.h"
#include "custom_cast.h"
#include "get_dll_dir.h"
#include "timestamp.h"
#include <string.h>

#define SYNCHRONI_WRITE_CHAR "f000ffe1-0451-4000-b000-000000000000"
#define SYNCHRONI_NOTIFY_CHAR "f000ffe2-0451-4000-b000-000000000000"




static void synchroni_adapter_1_on_scan_found(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((SynchroniBoard *)(board))->adapter_1_on_scan_found (adapter, peripheral);
}
static void synchroni_read_notification (simpleble_uuid_t service,
    simpleble_uuid_t characteristic, uint8_t *data, size_t size, void *board)
{
    if (size == SynchroniBoard::synchroni_handshaking_packet_size)
    {
        //((SynchroniBoard *)(board))->setSoftwareGain (data[1]);
        //((SynchroniBoard *)(board))->setHardwareGain (data[2]);
        //((SynchroniBoard *)(board))->setReferenceVoltage (((data[3] << 8) | data[4]));
    }
    else
    {
        //((SynchroniBoard *)(board))->read_data (service, characteristic, data, size, 0);
    }
}

SynchroniBoard::SynchroniBoard (struct BrainFlowInputParams params)
: BLELibBoard ((int)BoardIds::SYNCHRONI_BOARD,params)
{
    initialized = false;
    is_streaming = false;
}

SynchroniBoard::~SynchroniBoard()
{
    skip_logs = true;
    release_session();
}

int SynchroniBoard::prepare_session()
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
    synchroni_adapter = simpleble_adapter_get_handle (0);
    if(synchroni_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }
    simpleble_adapter_set_callback_on_scan_found (
        synchroni_adapter, ::synchroni_adapter_1_on_scan_found, (void *)this);
    if (!simpleble_adapter_is_bluetooth_enabled ())
    {
        safe_logger (spdlog::level::warn, "Bluetooth is disabled.");

    }
    simpleble_adapter_scan_start (synchroni_adapter);
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (
            lk, params.timeout * sec, [this] { return this->synchroni_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found Synchroni device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Synchroni Device niot found");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (synchroni_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        // for safety
        for (int i = 0; i < 3; i++)
        {
            if (simpleble_peripheral_connect (synchroni_peripheral) == SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::info, "Connected to Synchroni Device");
                res = (int)BrainFlowExitCodes::STATUS_OK;
                break;
            }
            else
            {
                safe_logger (
                    spdlog::level::warn, "Failed to connect to Synchroni Device: {}/3", i);
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
// Copied this workaround from issue in brainalive
#ifdef __linux__
        usleep (1000000);
#endif
    }
    bool control_characteristics_found = false;
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        size_t services_count = simpleble_peripheral_services_count (synchroni_peripheral);
        for (size_t i = 0; i < services_count; i++)
        {
            simpleble_service_t service;
            if (simpleble_peripheral_services_get (synchroni_peripheral, i, &service) !=
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
                    if (simpleble_peripheral_notify (synchroni_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::synchroni_read_notification,
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

int SynchroniBoard::start_stream(int buffer_size, const char * streamer_params)
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
        safe_logger (spdlog::level::debug, "Start command Send");
        is_streaming = true;
    }

    return res;
}

int SynchroniBoard::stop_stream ()
{
    if (synchroni_peripheral == NULL)
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

int SynchroniBoard::release_session ()
{
    if(initialized)
    {
        for(int i =0; i <2; i ++)
        {
            stop_stream ();
            // need to wait for notifications to stop triggered before unsubscribing, otherwise
            // macos fails inside simpleble with timeout
#ifdef _WIN32
            Sleep (2000);
#else
            sleep (2);
#endif
            if (simpleble_peripheral_unsubscribe (synchroni_peripheral,
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
            free_packages ();
            initialized = false;
        }
    }

    if(synchroni_peripheral != NULL){
        simpleble_adapter_release_handle (synchroni_adapter);
        synchroni_adapter = NULL;
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


    safe_logger (spdlog::level::trace, config[2]);
    std::vector<uint8_t> configVector(config.begin(), config.end());
    uint8_t *command = &configVector[0];
    if (simpleble_peripheral_write_command (synchroni_peripheral, write_characteristics.first,
            write_characteristics.second, command, sizeof (command)) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}


void SynchroniBoard::read_data(simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
        size_t size, int channel_num)
{
    if (size != synchroni_packet_size)
    {
        safe_logger (spdlog::level::warn, "unexpected number of bytes received: {}", size);
        return;
    }
    int num_rows = board_descr["default"]["num_rows"];
    double *package = new double[num_rows];

    for (int i = 0; i < num_rows; i++){
        int num_rows = board_descr["default"]["num_rows"];
            double *package = new double[num_rows];
        for (int i = 0; i < num_rows; i+=2)
        {
            package[i] = 0.0;
        }
        std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
        std::vector<int> accel_channels = board_descr["default"]["accel_channels"];
        std::vector<int> gyro_channels = board_descr["default"]["gyro_channels"];
        package[board_descr["default"]["package_num_channel"].get<int> ()] =
                data[synchroni_packet_size + i];

        for(int j = i + synchroni_axl_start_index, k =0; j <i +synchroni_axl_end_index;j += 2, k++)
        {
            package[accel_channels[k]] = (data[j] << 8) | data[j + 1];
            if (package[accel_channels[k]] > 32767)
                package[accel_channels[k]] = package[accel_channels[k]] - 65535;
        }
        
        for (int j = i + synchroni_gyro_start_index, k = 0; j < i + synchroni_gyro_end_index;
                 j += 2, k++)
            {
                package[gyro_channels[k]] = (data[j] << 8) | data[j + 1];
                if (package[gyro_channels[k]] > 32767)
                    package[gyro_channels[k]] = package[gyro_channels[k]] - 65535;
            }

        package[board_descr["default"]["marker_channel"].get<int> ()] =
            data[(synchroni_packet_index + 1) + i];
        package[board_descr["default"]["timestamp_channel"].get<int> ()] = get_timestamp ();

        push_package (&package[0]);
        
    }
    return;
}
void SynchroniBoard::adapter_1_on_scan_found(simpleble_adapter_t adapter, simpleble_peripheral_t peripheral)
{
    char *peripheral_indentified = simpleble_peripheral_identifier(peripheral);
    char *peripheral_address = simpleble_peripheral_address (peripheral);
    bool found = false;
    if(!params.mac_address.empty())
    {
        if (strcmp(peripheral_address, params.mac_address.c_str()) == 0){
            found = true;
        }
    }
    else
    {
        if (strncmp(peripheral_indentified, "PLACEHOLDER",7) == 0){
            found = true;
        }
    }
    safe_logger (spdlog::level::trace, "address {}", peripheral_address);
    simpleble_free (peripheral_address);
    safe_logger (spdlog::level::trace, "identifier {}", peripheral_indentified);
    simpleble_free (peripheral_indentified);
        if (found)
    {
        {
            std::lock_guard<std::mutex> lk (m);
            synchroni_peripheral = peripheral;
        }
        cv.notify_one ();
    }
    else
    {
        simpleble_peripheral_release_handle (peripheral);
    }
}