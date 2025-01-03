#include <algorithm>
#include <regex>

#include "custom_cast.h"
#include "muse.h"
#include "muse_constants.h"
#include "timestamp.h"

#include <iostream>

void adapter_on_scan_found (
    simpleble_adapter_t adapter, simpleble_peripheral_t peripheral, void *board)
{
    ((Muse *)(board))->adapter_on_scan_found (adapter, peripheral);
}

void peripheral_on_tp9 (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (peripheral, service, characteristic, data, size, 0);
}

void peripheral_on_af7 (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (peripheral, service, characteristic, data, size, 1);
}

void peripheral_on_af8 (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (peripheral, service, characteristic, data, size, 2);
}

void peripheral_on_tp10 (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (peripheral, service, characteristic, data, size, 3);
}

void peripheral_on_accel (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_accel (peripheral, service, characteristic, data, size);
}

void peripheral_on_gyro (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_gyro (peripheral, service, characteristic, data, size);
}

void peripheral_on_ppg0 (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_ppg (peripheral, service, characteristic, data, size, 0);
}

void peripheral_on_ppg1 (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_ppg (peripheral, service, characteristic, data, size, 1);
}

void peripheral_on_ppg2 (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_ppg (peripheral, service, characteristic, data, size, 2);
}

void peripheral_on_right_aux (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, void *board)
{
    ((Muse *)(board))->peripheral_on_eeg (peripheral, service, characteristic, data, size, 4);
}


Muse::Muse (int board_id, struct BrainFlowInputParams params) : BLELibBoard (board_id, params)
{
    initialized = false;
    muse_adapter = NULL;
    muse_peripheral = NULL;
    is_streaming = false;
    last_fifth_chan_timestamp = -1.0;
    last_ppg_timestamp = -1.0;
    last_eeg_timestamp = -1.0;
    last_aux_timestamp = -1.0;
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
        muse_adapter, ::adapter_on_scan_found, (void *)this);

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
        // for safety
        for (int i = 0; i < 3; i++)
        {
            if (simpleble_peripheral_connect (muse_peripheral) == SIMPLEBLE_SUCCESS)
            {
                safe_logger (spdlog::level::info, "Connected to Muse Device");
                res = (int)BrainFlowExitCodes::STATUS_OK;
                break;
            }
            else
            {
                safe_logger (spdlog::level::warn, "Failed to connect to Muse Device: {}/3", i);
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

                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_STREAM_TOGGLE) ==
                    0)
                {
                    control_characteristics = std::pair<simpleble_uuid_t, simpleble_uuid_t> (
                        service.uuid, service.characteristics[j].uuid);
                    control_characteristics_found = true;
                    safe_logger (spdlog::level::info, "found control characteristic");
                }
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_TP9) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_tp9,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_TP10) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_tp10,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_AF7) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_af7,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_AF8) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_af8,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_RIGHTAUX) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_right_aux,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_GYRO) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_gyro,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_ACCELEROMETER) ==
                    0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_accel,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_PPG0) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_ppg0,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_PPG1) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_ppg1,
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
                if (strcmp (service.characteristics[j].uuid.value, MUSE_GATT_ATTR_PPG2) == 0)
                {
                    if (simpleble_peripheral_notify (muse_peripheral, service.uuid,
                            service.characteristics[j].uuid, ::peripheral_on_ppg2,
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

    if ((res == (int)BrainFlowExitCodes::STATUS_OK) && (control_characteristics_found))
    {
        int eeg_buffer_size = board_descr["default"]["num_rows"].get<int> ();
        int aux_buffer_size = board_descr["auxiliary"]["num_rows"].get<int> ();
        current_default_buf.resize (12); // 12 eeg packages in single ble transaction
        new_eeg_data.resize (5);         // 5 eeg channels total
        current_aux_buf.resize (3);      // 3 samples in each message for gyro and accel
        for (int i = 0; i < 12; i++)
        {
            current_default_buf[i].resize (eeg_buffer_size);
            std::fill (current_default_buf[i].begin (), current_default_buf[i].end (), 0.0);
        }
        std::fill (new_eeg_data.begin (), new_eeg_data.end (), false);
        for (int i = 0; i < 3; i++)
        {
            current_aux_buf[i].resize (aux_buffer_size);
            std::fill (current_aux_buf[i].begin (), current_aux_buf[i].end (), 0.0);
        }
        // muse 2016 has no ppg
        if (board_id != (int)BoardIds::MUSE_2016_BOARD)
        {
            int anc_buffer_size = board_descr["ancillary"]["num_rows"].get<int> ();
            current_anc_buf.resize (6); // 6 ppg packages in single transaction
            for (int i = 0; i < 6; i++)
            {
                current_anc_buf[i].resize (anc_buffer_size);
                std::fill (current_anc_buf[i].begin (), current_anc_buf[i].end (), 0.0);
            }
            new_ppg_data.resize (3); // 3 ppg chars
            std::fill (new_ppg_data.begin (), new_ppg_data.end (), false);
        }

        initialized = true;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = config_board ("v1");
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
    }
    else
    {
        res = (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    }
    is_streaming = false;
    last_fifth_chan_timestamp = -1.0;
    last_ppg_timestamp = -1.0;
    last_eeg_timestamp = -1.0;
    last_aux_timestamp = -1.0;
    return res;
}

int Muse::release_session ()
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

    for (size_t i = 0; i < current_default_buf.size (); i++)
    {
        current_default_buf[i].clear ();
    }
    current_default_buf.clear ();
    new_eeg_data.clear ();
    for (size_t i = 0; i < current_aux_buf.size (); i++)
    {
        current_aux_buf[i].clear ();
    }
    current_aux_buf.clear ();
    for (size_t i = 0; i < current_anc_buf.size (); i++)
    {
        current_anc_buf[i].clear ();
    }
    current_anc_buf.clear ();
    new_ppg_data.clear ();

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
    if (simpleble_peripheral_write_command (muse_peripheral, control_characteristics.first,
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

void Muse::peripheral_on_eeg (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, size_t channel_num)
{
    std::lock_guard<std::mutex> callback_guard (callback_lock);
    if (size != 20)
    {
        safe_logger (spdlog::level::warn, "unknown size for eeg callback: {}", size);
        return;
    }

    /* 5th(aux) channel is off by default, need to enable p50 preset. Need to handle both cases, use
     * timestamps to determine if its on or not */
    if (channel_num == 4)
    {
        last_fifth_chan_timestamp = get_timestamp ();
    }
    new_eeg_data[channel_num] = true;

    std::vector<int> eeg_channels = board_descr["default"]["eeg_channels"];
    unsigned int package_num = data[0] * 256 + data[1];
    for (size_t i = 2, counter = 0; i < size; i += 3, counter += 2)
    {
        double val1 = data[i] << 4 | data[i + 1] >> 4;
        double val2 = (data[i + 1] & 0xF) << 8 | data[i + 2];
        val1 = (val1 - 0x800) * 125.0 / 256.0;
        val2 = (val2 - 0x800) * 125.0 / 256.0;
        // place optional aux channel to other channels
        if (channel_num == 4)
        {
            try
            {
                std::vector<int> other_channels = board_descr["default"]["other_channels"];
                current_default_buf[counter][other_channels[0]] = val1;
                current_default_buf[counter + 1][other_channels[0]] = val2;
            }
            catch (...)
            {
                safe_logger (spdlog::level::trace,
                    "no other_channels for this board"); // should not get here
            }
        }
        else
        {
            current_default_buf[counter][eeg_channels[channel_num]] = val1;
            current_default_buf[counter + 1][eeg_channels[channel_num]] = val2;
        }
        current_default_buf[counter][board_descr["default"]["package_num_channel"].get<int> ()] =
            package_num;
        current_default_buf[counter +
            1][board_descr["default"]["package_num_channel"].get<int> ()] = package_num;
    }

    int num_trues = 0;
    for (size_t i = 0; i < new_eeg_data.size (); i++)
    {
        if (new_eeg_data[i])
        {
            num_trues++;
        }
    }

    double current_timestamp = get_timestamp ();

    if ((num_trues == new_eeg_data.size ()) ||
        ((num_trues == new_eeg_data.size () - 1) &&
            (current_timestamp - last_fifth_chan_timestamp > 1)))
    {
        // skip one package to setup timestamp correction
        if (last_eeg_timestamp > 0)
        {
            double step = (current_timestamp - last_eeg_timestamp) / current_default_buf.size ();
            for (size_t i = 0; i < current_default_buf.size (); i++)
            {
                current_default_buf[i][board_descr["default"]["timestamp_channel"].get<int> ()] =
                    last_eeg_timestamp + step * (i + 1);
                push_package (&current_default_buf[i][0]);
            }
        }
        last_eeg_timestamp = current_timestamp;
        std::fill (new_eeg_data.begin (), new_eeg_data.end (), false);
    }
}

void Muse::peripheral_on_accel (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> callback_guard (callback_lock);
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
        current_aux_buf[i][board_descr["auxiliary"]["accel_channels"][0].get<int> ()] = accel_valx;
        current_aux_buf[i][board_descr["auxiliary"]["accel_channels"][1].get<int> ()] = accel_valy;
        current_aux_buf[i][board_descr["auxiliary"]["accel_channels"][2].get<int> ()] = accel_valz;
    }
}

void Muse::peripheral_on_gyro (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> callback_guard (callback_lock);
    if (size != 20)
    {
        safe_logger (spdlog::level::warn, "unknown size for gyro callback: {}", size);
        return;
    }

    unsigned int package_num = data[0] * 256 + data[1];
    double current_timestamp = get_timestamp ();

    for (int i = 0; i < 3; i++)
    {
        double gyro_valx = (double)cast_16bit_to_int32 ((unsigned char *)&data[2 + i * 6]) *
            MUSE_GYRO_SCALE_FACTOR;
        double gyro_valy = (double)cast_16bit_to_int32 ((unsigned char *)&data[4 + i * 6]) *
            MUSE_GYRO_SCALE_FACTOR;
        double gyro_valz = (double)cast_16bit_to_int32 ((unsigned char *)&data[6 + i * 6]) *
            MUSE_GYRO_SCALE_FACTOR;
        current_aux_buf[i][board_descr["auxiliary"]["gyro_channels"][0].get<int> ()] = gyro_valx;
        current_aux_buf[i][board_descr["auxiliary"]["gyro_channels"][1].get<int> ()] = gyro_valy;
        current_aux_buf[i][board_descr["auxiliary"]["gyro_channels"][2].get<int> ()] = gyro_valz;
        current_aux_buf[i][board_descr["auxiliary"]["package_num_channel"].get<int> ()] =
            (double)package_num;
    }

    if (last_aux_timestamp > 0)
    {
        double step = (current_timestamp - last_aux_timestamp) / current_aux_buf.size ();
        // push aux packages from gyro callback
        for (size_t i = 0; i < current_aux_buf.size (); i++)
        {
            current_aux_buf[i][board_descr["auxiliary"]["timestamp_channel"].get<int> ()] =
                last_aux_timestamp + step * (i + 1);
            push_package (&current_aux_buf[i][0], (int)BrainFlowPresets::AUXILIARY_PRESET);
        }
    }
    last_aux_timestamp = current_timestamp;
}

void Muse::peripheral_on_ppg (simpleble_peripheral_t peripheral, simpleble_uuid_t service,
    simpleble_uuid_t characteristic, const uint8_t *data, size_t size, size_t ppg_num)
{
    std::lock_guard<std::mutex> callback_guard (callback_lock);
    if (size != 20)
    {
        safe_logger (spdlog::level::warn, "unknown size for ppg callback: {}", size);
        return;
    }
    unsigned int package_num = data[0] * 256 + data[1];
    new_ppg_data[ppg_num] = true;
    std::vector<int> ppg_channels = board_descr["ancillary"]["ppg_channels"];
    // format is: 2 bytes for package num, 6 int24 values for actual data
    for (int i = 0; i < 6; i++)
    {
        double ppg_val = (double)cast_24bit_to_int32 ((unsigned char *)&data[2 + i * 3]);
        current_anc_buf[i][ppg_channels[ppg_num]] = ppg_val;
    }
    int num_trues = 0;
    for (size_t i = 0; i < new_ppg_data.size (); i++)
    {
        if (new_ppg_data[i])
        {
            num_trues++;
        }
    }

    double current_timestamp = get_timestamp ();

    if (num_trues == new_ppg_data.size () - 1) // actually it streams only 2 of 3 ppg data types and
                                               // I am not sure that these 2 are freezed
    {
        // skip one package to setup timestamp correction
        if (last_ppg_timestamp > 0)
        {
            double step = (current_timestamp - last_ppg_timestamp) / current_anc_buf.size ();
            for (size_t i = 0; i < current_anc_buf.size (); i++)
            {
                current_anc_buf[i][board_descr["ancillary"]["timestamp_channel"].get<int> ()] =
                    last_ppg_timestamp + step * (i + 1);
                push_package (&current_anc_buf[i][0], (int)BrainFlowPresets::ANCILLARY_PRESET);
            }
        }
        last_ppg_timestamp = current_timestamp;
        std::fill (new_ppg_data.begin (), new_ppg_data.end (), false);
    }
}
