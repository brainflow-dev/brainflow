#include "muse_s.h"
#include "muse_constants.h"

void adapter_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((MuseS *)(board))->adapter_on_scan_found (adapter, peripheral);
}

void peripheral_on_tp9 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((MuseS *)(board))->peripheral_on_tp9 (service, characteristic, data, size);
}

void peripheral_on_af7 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((MuseS *)(board))->peripheral_on_af7 (service, characteristic, data, size);
}

void peripheral_on_af8 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((MuseS *)(board))->peripheral_on_af8 (service, characteristic, data, size);
}

void peripheral_on_tp10 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((MuseS *)(board))->peripheral_on_tp10 (service, characteristic, data, size);
}

void peripheral_on_accel (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((MuseS *)(board))->peripheral_on_accel (service, characteristic, data, size);
}

void peripheral_on_gyro (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((MuseS *)(board))->peripheral_on_gyro (service, characteristic, data, size);
}

MuseS::MuseS (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::MUSE_S_BOARD, params)
{
    initialized = false;
    muse_adapter = NULL;
    muse_peripheral = NULL;
}

MuseS::~MuseS ()
{
    skip_logs = true;
    release_session ();
}

int MuseS::prepare_session ()
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

    muse_adapter = simpleble_adapter_get_handle (0);
    if (muse_adapter == NULL)
    {
        safe_logger (spdlog::level::err, "Adapter is NULL");
        return (int)BrainFlowExitCodes::UNABLE_TO_OPEN_PORT_ERROR;
    }

    simpleble_adapter_set_callback_on_scan_found (
        muse_adapter, ::adapter_on_scan_found, (void *)this);

    simpleble_adapter_scan_start (muse_adapter);
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::unique_lock<std::mutex> lk (m);
    auto sec = std::chrono::seconds (1);
    if (cv.wait_for (lk, params.timeout * sec, [this] { return this->muse_peripheral != NULL; }))
    {
        safe_logger (spdlog::level::info, "Found Muse device");
    }
    else
    {
        safe_logger (spdlog::level::err, "Failed to find Muse Device");
        res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    simpleble_adapter_scan_stop (muse_adapter);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (simpleble_peripheral_connect (muse_peripheral) == SIMPLEBLE_SUCCESS)
        {
            safe_logger (spdlog::level::info, "Connected to Muse Device");
        }
        else
        {
            safe_logger (spdlog::level::err, "Failed to connect to Muse Device");
            res = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        }
    }

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
                    service.characteristics[j].value);

                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_STREAM_TOGGLE) == 0)
                {
                    control_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j]);
                    control_characteristics_found = true;
                    safe_logger (spdlog::level::info, "found control characteristic");
                }
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_TP9) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_tp9,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j]));
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Failed to notify for {} {}",
                            service.uuid.value, service.characteristics[j].value);
                        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
                    }
                }
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_TP10) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_tp10,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j]));
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Failed to notify for {} {}",
                            service.uuid.value, service.characteristics[j].value);
                        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
                    }
                }
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_AF7) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_af7,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j]));
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Failed to notify for {} {}",
                            service.uuid.value, service.characteristics[j].value);
                        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
                    }
                }
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_AF8) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_af8,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j]));
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Failed to notify for {} {}",
                            service.uuid.value, service.characteristics[j].value);
                        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
                    }
                }
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_GYRO) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_gyro,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j]));
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Failed to notify for {} {}",
                            service.uuid.value, service.characteristics[j].value);
                        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
                    }
                }
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_ACCELEROMETER) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_accel,
                            (void *)this) == SIMPLEBLE_SUCCESS)
                    {
                        notified_characteristics.push_back (
                            std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                                service.uuid, service.characteristics[j]));
                    }
                    else
                    {
                        safe_logger (spdlog::level::err, "Failed to notify for {} {}",
                            service.uuid.value, service.characteristics[j].value);
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

int MuseS::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    res = config_board ("p21");
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("d");
    }

    return res;
}

int MuseS::stop_stream ()
{
    if (muse_peripheral == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = config_board ("h");
    for (auto notified : notified_characteristics)
    {
        if (simpleble_peripheral_unsubscribe (muse_peripheral, notified.first, notified.second) !=
            SIMPLEBLE_SUCCESS)
        {
            safe_logger (spdlog::level::err, "failed to unsubscribe for {} {}",
                notified.first.value, notified.second.value);
            res = (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
        }
    }
    return res;
}

int MuseS::release_session ()
{
    if (initialized)
    {
        stop_stream ();
        free_packages ();
        initialized = false;
    }
    if (muse_adapter != NULL)
    {
        simpleble_adapter_release_handle (muse_adapter);
        muse_adapter = NULL;
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

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MuseS::config_board (std::string config, std::string &response)
{
    return config_board (config);
}

int MuseS::config_board (std::string config)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    uint8_t command[16];
    size_t len = config.size ();
    command[0] = (uint8_t)len + 1;
    for (size_t i = 0; i < len; i++)
    {
        command[i + 1] = uint8_t (config[i]);
    }
    command[len + 1] = 10;
    if (simpleble_peripheral_write_request (muse_peripheral, control_characteristics.first,
            control_characteristics.second, command, len + 2) != SIMPLEBLE_SUCCESS)
    {
        safe_logger (spdlog::level::err, "failed to send command {} to device", config.c_str ());
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void MuseS::adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral)
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
            if (strncmp (peripheral_identified, "Muse", 4) == 0)
            {
                found = true;
            }
        }
    }

    safe_logger (spdlog::level::info, "address {}", peripheral_address);
    simpleble_free (peripheral_address);
    safe_logger (spdlog::level::info, "identifier {}", peripheral_identified);
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

void MuseS::peripheral_on_tp9 (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "notification for tp9");
}

void MuseS::peripheral_on_af7 (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "notification for af7");
}

void MuseS::peripheral_on_af8 (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "notification for af8");
}

void MuseS::peripheral_on_tp10 (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "notification for tp10");
}

void MuseS::peripheral_on_accel (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "notification for accel");
}

void MuseS::peripheral_on_gyro (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    safe_logger (spdlog::level::trace, "notification for gyro");
}
