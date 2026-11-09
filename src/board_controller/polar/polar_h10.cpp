#include "polar_h10.h"
#include "polar_protocol.h"
#include "timestamp.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <limits>

namespace
{
    const simpleble_uuid_t pmd_service = {"fb005c80-02e7-f387-1cad-8acd2d8df0c8"};
    const simpleble_uuid_t pmd_control = {"fb005c81-02e7-f387-1cad-8acd2d8df0c8"};
    const simpleble_uuid_t pmd_data = {"fb005c82-02e7-f387-1cad-8acd2d8df0c8"};
    const simpleble_uuid_t hr_service = {"0000180d-0000-1000-8000-00805f9b34fb"};
    const simpleble_uuid_t hr_data = {"00002a37-0000-1000-8000-00805f9b34fb"};
    const simpleble_uuid_t battery_service = {"0000180f-0000-1000-8000-00805f9b34fb"};
    const simpleble_uuid_t battery_level = {"00002a19-0000-1000-8000-00805f9b34fb"};

    std::string lower (std::string value)
    {
        std::transform (value.begin (), value.end (), value.begin (),
            [] (unsigned char c) { return char (std::tolower (c)); });
        return value;
    }
}

PolarH10::PolarH10 (struct BrainFlowInputParams params)
    : BLELibBoard ((int)BoardIds::POLAR_H10_BOARD, params)
    , adapter (nullptr)
    , peripheral (nullptr)
    , control_subscribed (false)
    , data_subscribed (false)
    , hr_subscribed (false)
    , initialized (false)
    , streaming (false)
    , accept_data (false)
    , ready (false)
    , awaiting (false)
    , operation (0)
    , control_result (-1)
    , ecg_counter (0)
    , hr_counter (0)
    , last_timestamp (0)
    , battery (std::numeric_limits<double>::quiet_NaN ())
{
}

PolarH10::~PolarH10 ()
{
    skip_logs = true;
    release_session ();
}

int PolarH10::connect_device ()
{
    if (!init_dll_loader () || (simpleble_adapter_get_count () == 0))
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    adapter = simpleble_adapter_get_handle (0);
    if (!adapter)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    auto found = [] (simpleble_adapter_t, simpleble_peripheral_t candidate, void *context)
    {
        auto self = static_cast<PolarH10 *> (context);
        std::lock_guard<std::mutex> lock (self->mutex);
        // No BLE calls here: scan_for holds BLELibBoard's loader mutex.
        self->discovered.push_back (candidate);
    };
    if (simpleble_adapter_set_callback_on_scan_found (adapter, found, this) != SIMPLEBLE_SUCCESS ||
        simpleble_adapter_scan_for (adapter, params.timeout * 1000) != SIMPLEBLE_SUCCESS)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    std::vector<simpleble_peripheral_t> candidates;
    {
        std::lock_guard<std::mutex> lock (mutex);
        candidates.swap (discovered);
    }
    for (auto candidate : candidates)
    {
        char *id = simpleble_peripheral_identifier (candidate);
        char *address = simpleble_peripheral_address (candidate);
        std::string name = id ? id : "";
        std::string mac = address ? address : "";
        simpleble_free (id);
        simpleble_free (address);
        bool match = name.find ("Polar H10 ") == 0 &&
            (params.mac_address.empty () || lower (params.mac_address) == lower (mac)) &&
            (params.serial_number.empty () ||
                lower (name.substr (10)) == lower (params.serial_number));
        if (match && !peripheral)
            peripheral = candidate;
        else
            simpleble_peripheral_release_handle (candidate);
    }
    if (!peripheral || simpleble_peripheral_connect (peripheral) != SIMPLEBLE_SUCCESS)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    bool has_control = false, indicate = false, has_data = false, has_hr = false;
    for (size_t i = 0; i < simpleble_peripheral_services_count (peripheral); ++i)
    {
        simpleble_service_t service;
        if (simpleble_peripheral_services_get (peripheral, i, &service) != SIMPLEBLE_SUCCESS)
            return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
        for (size_t j = 0; j < service.characteristic_count; ++j)
        {
            const auto &characteristic = service.characteristics[j];
            std::string uuid = lower (characteristic.uuid.value);
            if (lower (service.uuid.value) == pmd_service.value && uuid == pmd_control.value)
            {
                has_control = characteristic.can_write_request &&
                    (characteristic.can_indicate || characteristic.can_notify);
                indicate = characteristic.can_indicate;
            }
            if (lower (service.uuid.value) == pmd_service.value && uuid == pmd_data.value)
                has_data = characteristic.can_notify;
            if (lower (service.uuid.value) == hr_service.value && uuid == hr_data.value)
                has_hr = characteristic.can_notify;
        }
    }
    if (!has_control || !has_data || !has_hr)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    auto control_callback = [] (simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t,
                                const uint8_t *data, size_t size, void *context)
    { static_cast<PolarH10 *> (context)->on_control (data, size); };
    control_subscribed = (indicate ? simpleble_peripheral_indicate (peripheral, pmd_service,
                                         pmd_control, control_callback, this) :
                                     simpleble_peripheral_notify (peripheral, pmd_service,
                                         pmd_control, control_callback, this)) == SIMPLEBLE_SUCCESS;
    if (!control_subscribed)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    data_subscribed = simpleble_peripheral_notify (
                          peripheral, pmd_service, pmd_data,
                          [] (simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t,
                              const uint8_t *data, size_t size, void *context)
                          { static_cast<PolarH10 *> (context)->on_ecg (data, size); },
                          this) == SIMPLEBLE_SUCCESS;
    hr_subscribed = simpleble_peripheral_notify (
                        peripheral, hr_service, hr_data,
                        [] (simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t,
                            const uint8_t *data, size_t size, void *context)
                        { static_cast<PolarH10 *> (context)->on_heart_rate (data, size); },
                        this) == SIMPLEBLE_SUCCESS;
    if (!data_subscribed || !hr_subscribed)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    uint8_t *level = nullptr;
    size_t size = 0;
    if (simpleble_peripheral_read (peripheral, battery_service, battery_level, &level, &size) ==
        SIMPLEBLE_SUCCESS)
    {
        if ((size == 1) && level && (level[0] <= 100))
            battery = level[0];
    }
    simpleble_free (level);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

bool PolarH10::write_control (const std::vector<uint8_t> &command)
{
    auto bytes = command;
    return simpleble_peripheral_write_request (peripheral, pmd_service, pmd_control, bytes.data (),
               bytes.size ()) == SIMPLEBLE_SUCCESS;
}

void PolarH10::disconnect_device ()
{
    if (peripheral)
    {
        if (hr_subscribed)
            simpleble_peripheral_unsubscribe (peripheral, hr_service, hr_data);
        if (data_subscribed)
            simpleble_peripheral_unsubscribe (peripheral, pmd_service, pmd_data);
        if (control_subscribed)
            simpleble_peripheral_unsubscribe (peripheral, pmd_service, pmd_control);
        simpleble_peripheral_disconnect (peripheral);
        simpleble_peripheral_release_handle (peripheral);
        peripheral = nullptr;
    }
    hr_subscribed = data_subscribed = control_subscribed = false;
    if (adapter)
    {
        simpleble_adapter_release_handle (adapter);
        adapter = nullptr;
    }
    std::vector<simpleble_peripheral_t> remaining;
    {
        std::lock_guard<std::mutex> lock (mutex);
        remaining.swap (discovered);
    }
    for (auto candidate : remaining)
        simpleble_peripheral_release_handle (candidate);
}

int PolarH10::transact (const std::vector<uint8_t> &command, std::vector<uint8_t> &response)
{
    {
        std::lock_guard<std::mutex> lock (mutex);
        operation = command[0];
        awaiting = true;
        control_result = -1;
        control_parameters.clear ();
    }
    if (!write_control (command))
    {
        std::lock_guard<std::mutex> lock (mutex);
        awaiting = false;
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    std::unique_lock<std::mutex> lock (mutex);
    if (!received.wait_for (
            lock, std::chrono::seconds (params.timeout), [this] { return !awaiting; }))
    {
        awaiting = false;
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
    if (control_result != 0)
    {
        safe_logger (spdlog::level::err, "Polar PMD command {} rejected with status {}", operation,
            control_result);
        return (int)BrainFlowExitCodes::BOARD_WRITE_ERROR;
    }
    response = control_parameters;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void PolarH10::on_control (const uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (!awaiting || (size < 4) || (data[0] != 0xf0) || (data[1] != operation) || (data[2] != 0))
        return;
    control_result = data[3];
    if (control_result == 0 && size >= 5)
    {
        if (control_parameters.size () + size - 5 > 2048 || data[4] > 1)
            control_result = -2;
        else
        {
            control_parameters.insert (control_parameters.end (), data + 5, data + size);
            if (data[4] != 0)
                return;
        }
    }
    awaiting = false;
    received.notify_all ();
}

int PolarH10::prepare_session ()
{
    if (initialized)
        return (int)BrainFlowExitCodes::STATUS_OK;
    if ((params.timeout < 0) || (params.timeout > 60))
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    if (params.timeout == 0)
        params.timeout = 5;
    battery = std::numeric_limits<double>::quiet_NaN ();
    int result = connect_device ();
    if (result == (int)BrainFlowExitCodes::STATUS_OK)
    {
        std::vector<uint8_t> settings;
        result = transact ({1, 0}, settings);
        if (result == (int)BrainFlowExitCodes::STATUS_OK && !polar::supports_ecg_profile (settings))
            result = (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    }
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
        disconnect_device ();
    else
        initialized = true;
    return result;
}

int PolarH10::start_stream (int buffer_size, const char *streamer_params)
{
    if (!initialized)
        return (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR;
    if (streaming)
        return (int)BrainFlowExitCodes::STREAM_ALREADY_RUN_ERROR;
    int result = prepare_for_acquisition (buffer_size, streamer_params);
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
        return result;
    std::vector<uint8_t> response;
    result = transact ({2, 0, 0, 1, 130, 0, 1, 1, 14, 0}, response);
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
    {
        disconnect_device ();
        initialized = false;
        return result;
    }
    streaming = true;
    std::unique_lock<std::mutex> lock (mutex);
    ecg_counter = hr_counter = 0;
    last_timestamp = 0;
    ready = false;
    accept_data = true;
    bool success =
        received.wait_for (lock, std::chrono::seconds (params.timeout), [this] { return ready; });
    lock.unlock ();
    if (!success)
    {
        stop_stream ();
        return (int)BrainFlowExitCodes::SYNC_TIMEOUT_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

void PolarH10::on_ecg (const uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (!accept_data)
        return;
    polar::Ecg frame;
    if (!polar::ecg (data, size, frame))
        return;
    if (last_timestamp && frame.timestamp <= last_timestamp)
        return;
    last_timestamp = frame.timestamp;
    double timestamp = get_timestamp ();
    for (size_t i = 0; i < frame.samples.size (); ++i)
    {
        double offset = (frame.samples.size () - 1 - i) / 130.0;
        double package[6] = {double (ecg_counter++), double (frame.samples[i]),
            frame.timestamp / 1e9 - offset, battery, timestamp - offset, 0};
        push_package (package);
    }
    ready = true;
    received.notify_all ();
}

void PolarH10::on_heart_rate (const uint8_t *data, size_t size)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (!accept_data)
        return;
    polar::HeartRate measurement;
    if (!polar::heart_rate (data, size, measurement))
        return;
    double timestamp = get_timestamp ();
    if (measurement.rr_ms.empty ())
        measurement.rr_ms.push_back (std::numeric_limits<double>::quiet_NaN ());
    double remaining = 0;
    for (size_t i = 1; i < measurement.rr_ms.size (); ++i)
        remaining += measurement.rr_ms[i] / 1000.0;
    for (size_t i = 0; i < measurement.rr_ms.size (); ++i)
    {
        double package[6] = {double (hr_counter++), measurement.bpm, measurement.rr_ms[i],
            measurement.contact, timestamp - remaining, 0};
        push_package (package, (int)BrainFlowPresets::AUXILIARY_PRESET);
        if (i + 1 < measurement.rr_ms.size ())
            remaining -= measurement.rr_ms[i + 1] / 1000.0;
    }
}

int PolarH10::stop_stream ()
{
    if (!streaming)
        return (int)BrainFlowExitCodes::STREAM_THREAD_IS_NOT_RUNNING;
    {
        std::lock_guard<std::mutex> lock (mutex);
        accept_data = false;
    }
    std::vector<uint8_t> response;
    int result = transact ({3, 0}, response);
    streaming = false;
    if (result != (int)BrainFlowExitCodes::STATUS_OK)
    {
        disconnect_device ();
        initialized = false;
    }
    return result;
}

int PolarH10::release_session ()
{
    if (streaming)
        stop_stream ();
    {
        std::lock_guard<std::mutex> lock (mutex);
        accept_data = false;
    }
    disconnect_device ();
    initialized = false;
    free_packages ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int PolarH10::config_board (std::string, std::string &)
{
    return (int)BrainFlowExitCodes::UNSUPPORTED_BOARD_ERROR;
}
