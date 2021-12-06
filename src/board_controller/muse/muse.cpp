#include <algorithm>

#include "custom_cast.h"
#include "muse.h"
#include "muse_constants.h"
#include "timestamp.h"

void adapter_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((Muse *)(board))->adapter_on_scan_found (adapter, peripheral);
}

void peripheral_on_tp9 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (service, characteristic, data, size, 0);
}

void peripheral_on_af7 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (service, characteristic, data, size, 1);
}

void peripheral_on_af8 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (service, characteristic, data, size, 2);
}

void peripheral_on_tp10 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (service, characteristic, data, size, 3);
}

void peripheral_on_accel (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_accel (service, characteristic, data, size);
}

void peripheral_on_gyro (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_gyro (service, characteristic, data, size);
}

void peripheral_on_ppg0 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_ppg (service, characteristic, data, size, 0);
}

void peripheral_on_ppg1 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_ppg (service, characteristic, data, size, 1);
}

void peripheral_on_ppg2 (simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data,
    size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_ppg (service, characteristic, data, size, 2);
}


Muse::Muse (int board_id, struct BrainFlowInputParams params) : BLELibBoard (board_id, params)
{
    initialized = false;
    muse_adapter = NULL;
    muse_peripheral = NULL;
    is_streaming = false;
    current_accel_pos = 0;
    current_gyro_pos = 0;
    current_ppg_pos = 0;
}

Muse::~Muse ()
{
    skip_logs = true;
    release_session ();
}

int Muse::prepare_session ()
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

// https://github.com/OpenBluetoothToolbox/SimpleBLE/issues/26#issuecomment-955606799
#ifdef __linux__
    usleep (1000000);
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
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_PPG0) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_ppg0,
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
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_PPG1) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_ppg1,
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
                if (strcmp (service.characteristics[j].value, MUSE_GATT_ATTR_PPG2) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j], ::peripheral_on_ppg2,
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
        int buffer_size = board_descr["num_rows"].get<int> ();
        current_buf.resize (12); // 12 eeg packages in single ble transaction
        new_eeg_data.resize (4); // 4 eeg channels total
        current_gyro_pos = 0;
        current_accel_pos = 0;
        current_ppg_pos = 0;
        for (int i = 0; i < 12; i++)
        {
            current_buf[i].resize (buffer_size);
            std::fill (current_buf[i].begin (), current_buf[i].end (), 0.0);
            std::fill (new_eeg_data.begin (), new_eeg_data.end (), false);
        }
        last_timestamp = -1.0;
        initialized = true;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("p21");
    }
    else
    {
        release_session ();
    }

    return res;
}

int Muse::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }

    int res = prepare_for_acquisition (buffer_size, streamer_params);
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("d");
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        is_streaming = true;
    }

    return res;
}

int Muse::stop_stream ()
{
    if (muse_peripheral == NULL)
    {
        return (int)BrainFlowExitCodes::BOARD_NOT_CREATED_ERROR;
    }
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (is_streaming)
    {
        res = config_board ("h");
        for (auto notified : notified_characteristics)
        {
            if (simpleble_peripheral_unsubscribe (
                    muse_peripheral, notified.first, notified.second) != SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::err, "failed to unsubscribe for {} {}",
                    notified.first.value, notified.second.value);
                res = (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
            }
        }
    }
    else
    {
        res = (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    is_streaming = false;
    return res;
}

int Muse::release_session ()
{
    if (initialized)
    {
        stop_stream ();
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

    for (size_t i = 0; i < current_buf.size (); i++)
    {
        current_buf[i].clear ();
    }
    current_buf.clear ();
    new_eeg_data.clear ();
    current_gyro_pos = 0;
    current_accel_pos = 0;
    current_ppg_pos = 0;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int Muse::config_board (std::string config, std::string &response)
{
    return config_board (config);
}

int Muse::config_board (std::string config)
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

void Muse::adapter_on_scan_found (simpleble_adapter_t adapter, simpleble_peripheral_t peripheral)
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

void Muse::peripheral_on_eeg (simpleble_uuid_t service, simpleble_uuid_t characteristic,
    uint8_t *data, size_t size, size_t channel_num)
{
    std::lock_guard<std::mutex> lock (callback_lock);
    if (size != 20)
    {
        safe_logger (spdlog::level::warn, "unknown size for eeg callback: {}", size);
        return;
    }
    new_eeg_data[channel_num] = true;

    std::vector<int> eeg_channels = board_descr["eeg_channels"];
    unsigned int package_num = data[0] * 256 + data[1];
    for (size_t i = 2, counter = 0; i < size; i += 3, counter += 2)
    {
        double val1 = data[i] << 4 | data[i + 1] >> 4;
        double val2 = (data[i + 1] & 0xF) << 8 | data[i + 2];
        val1 = (val1 - 0x800) * 125.0 / 256.0;
        val2 = (val2 - 0x800) * 125.0 / 256.0;
        current_buf[counter][eeg_channels[channel_num]] = val1;
        current_buf[counter + 1][eeg_channels[channel_num]] = val2;
        current_buf[counter][board_descr["package_num_channel"].get<int> ()] = package_num;
        current_buf[counter + 1][board_descr["package_num_channel"].get<int> ()] = package_num;
    }

    int num_trues = 0;
    for (size_t i = 0; i < new_eeg_data.size (); i++)
    {
        if (new_eeg_data[i])
        {
            num_trues++;
        }
    }
    if (num_trues == 1)
    {
        double timestamp = get_timestamp ();
        if (last_timestamp < 0)
        {
            last_timestamp = timestamp;
            return;
        }
        double step = (timestamp - last_timestamp) / current_buf.size ();
        last_timestamp = timestamp;
        for (size_t i = 0; i < current_buf.size (); i++)
        {
            current_buf[current_buf.size () - 1 - i][board_descr["timestamp_channel"].get<int> ()] =
                timestamp - i * step;
        }
    }

    if (num_trues == new_eeg_data.size ())
    {
        for (size_t i = 0; i < current_buf.size (); i++)
        {
            if (current_buf[i][board_descr["timestamp_channel"].get<int> ()] >
                1.0) // skip first package to set timestamp
            {
                push_package (&current_buf[i][0]);
            }
        }
        std::fill (new_eeg_data.begin (), new_eeg_data.end (), false);
    }
}

void Muse::peripheral_on_accel (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> lock (callback_lock);
    if (size != 20)
    {
        safe_logger (spdlog::level::warn, "unknown size for accel callback: {}", size);
        return;
    }
    for (int i = 0; i < 3; i++)
    {
        double accel_valx = (double)cast_16bit_to_int32 ((unsigned char *)&data[2 + i * 6]) / 16384;
        double accel_valy = (double)cast_16bit_to_int32 ((unsigned char *)&data[4 + i * 6]) / 16384;
        double accel_valz = (double)cast_16bit_to_int32 ((unsigned char *)&data[6 + i * 6]) / 16384;
        for (int j = 0; j < 4; j++)
        {
            int pos = (current_accel_pos + i * 4 + j) % 12;
            current_buf[pos][board_descr["accel_channels"][0].get<int> ()] = accel_valx;
            current_buf[pos][board_descr["accel_channels"][1].get<int> ()] = accel_valy;
            current_buf[pos][board_descr["accel_channels"][2].get<int> ()] = accel_valz;
        }
    }
    current_accel_pos += 4;
}

void Muse::peripheral_on_gyro (
    simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> lock (callback_lock);
    if (size != 20)
    {
        safe_logger (spdlog::level::warn, "unknown size for gyro callback: {}", size);
        return;
    }

    for (int i = 0; i < 3; i++)
    {
        double gyro_valx = (double)cast_16bit_to_int32 ((unsigned char *)&data[2 + i * 6]) *
            MUSE_GYRO_SCALE_FACTOR;
        double gyro_valy = (double)cast_16bit_to_int32 ((unsigned char *)&data[4 + i * 6]) *
            MUSE_GYRO_SCALE_FACTOR;
        double gyro_valz = (double)cast_16bit_to_int32 ((unsigned char *)&data[6 + i * 6]) *
            MUSE_GYRO_SCALE_FACTOR;

        for (int j = 0; j < 4; j++)
        {
            int pos = (current_gyro_pos + i * 4 + j) % 12;
            current_buf[pos][board_descr["gyro_channels"][0].get<int> ()] = gyro_valx;
            current_buf[pos][board_descr["gyro_channels"][1].get<int> ()] = gyro_valy;
            current_buf[pos][board_descr["gyro_channels"][2].get<int> ()] = gyro_valz;
        }
    }
    current_gyro_pos += 4;
}

void Muse::peripheral_on_ppg (simpleble_uuid_t service, simpleble_uuid_t characteristic,
    uint8_t *data, size_t size, size_t ppg_num)
{
    std::lock_guard<std::mutex> lock (callback_lock);
    if (size != 20)
    {
        safe_logger (spdlog::level::warn, "unknown size for ppg callback: {}", size);
        return;
    }

    std::vector<int> ppg_channels = board_descr["ppg_channels"];
    // format is: 2 bytes for package num, 6 int24 values for actual data
    for (int i = 0; i < 6; i++)
    {
        double ppg_val = (double)cast_24bit_to_int32 ((unsigned char *)&data[2 + i * 3]);
        for (int j = 0; j < 2; j++)
        {
            int pos = (current_ppg_pos + i * 2 + j) % 12;
            current_buf[pos][ppg_channels[ppg_num]] = ppg_val;
        }
    }
    current_ppg_pos += 2;
}
