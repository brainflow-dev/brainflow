#include <simpleble_c/peripheral.h>

#include <simpleble/PeripheralSafe.h>

#include <cstring>
#include <map>

std::map<simpleble_peripheral_t, std::map<std::string, SimpleBLE::ByteArray>> cache_read_data;
std::map<simpleble_peripheral_t, std::vector<SimpleBLE::BluetoothService>> cache_bluetooth_services;
std::map<simpleble_peripheral_t, std::vector<std::pair<uint16_t, SimpleBLE::ByteArray>>> cache_manufacturer_data;

const char* simpleble_peripheral_identifier(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    // TODO: This is a memory leak.
    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    return peripheral->identifier().value_or("").c_str();
}

const char* simpleble_peripheral_address(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    // TODO: This is a memory leak.
    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    return peripheral->address().value_or("").c_str();
}

simpleble_err_t simpleble_peripheral_connect(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    return peripheral->connect() ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_disconnect(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    return peripheral->disconnect() ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_is_connected(simpleble_peripheral_t handle, bool* connected) {
    if (handle == nullptr || connected == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    std::optional<bool> is_connected = peripheral->is_connected();
    *connected = is_connected.value_or(false);
    return is_connected.has_value() ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_is_connectable(simpleble_peripheral_t handle, bool* connectable) {
    if (handle == nullptr || connectable == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    std::optional<bool> is_connectable = peripheral->is_connectable();
    *connectable = is_connectable.value_or(false);
    return is_connectable.has_value() ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_services(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    auto services = peripheral->services();
    if (services.has_value()) {
        cache_bluetooth_services[handle] = services.value();
        return SIMPLEBLE_SUCCESS;
    } else {
        return SIMPLEBLE_FAILURE;
    }
}

size_t simpleble_peripheral_services_count(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    return cache_bluetooth_services[handle].size();
}

simpleble_err_t simpleble_peripheral_services_get(simpleble_peripheral_t handle, size_t index,
                                                  simpleble_service_t* services) {
    if (handle == nullptr || services == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    if (cache_bluetooth_services.find(handle) == cache_bluetooth_services.end()) {
        return SIMPLEBLE_FAILURE;
    }

    if (index >= cache_bluetooth_services[handle].size()) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::BluetoothService& service = cache_bluetooth_services[handle][index];

    memcpy(services->uuid.value, service.uuid.c_str(), SIMPLEBLE_UUID_STR_LEN);
    services->characteristic_count = service.characteristics.size();

    if (services->characteristic_count > SIMPLEBLE_CHARACTERISTIC_MAX_COUNT) {
        services->characteristic_count = SIMPLEBLE_CHARACTERISTIC_MAX_COUNT;
    }

    for (size_t i = 0; i < services->characteristic_count; i++) {
        memcpy(services->characteristics[i].value, service.characteristics[i].c_str(), SIMPLEBLE_UUID_STR_LEN);
    }

    return SIMPLEBLE_SUCCESS;
}

simpleble_err_t simpleble_peripheral_manufacturer_data(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    auto manufacturer_data = peripheral->manufacturer_data();
    if (manufacturer_data.has_value()) {
        for (auto& [manufacturer_id, data] : manufacturer_data.value()) {
            cache_manufacturer_data[handle].push_back({manufacturer_id, data});
        }
        return SIMPLEBLE_SUCCESS;
    } else {
        return SIMPLEBLE_FAILURE;
    }
}

size_t simpleble_peripheral_manufacturer_data_count(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    return cache_manufacturer_data[handle].size();
}

simpleble_err_t simpleble_peripheral_manufacturer_data_get(simpleble_peripheral_t handle, size_t index,
                                                           simpleble_manufacturer_data_t* manufacturer_data) {
    if (handle == nullptr || manufacturer_data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    if (cache_manufacturer_data.find(handle) == cache_manufacturer_data.end()) {
        return SIMPLEBLE_FAILURE;
    }

    if (index >= cache_manufacturer_data[handle].size()) {
        return SIMPLEBLE_FAILURE;
    }

    auto& cached_manufacturer_data = cache_manufacturer_data[handle][index];

    manufacturer_data->manufacturer_id = cached_manufacturer_data.first;
    manufacturer_data->data_length = cached_manufacturer_data.second.size();
    memcpy(manufacturer_data->data, cached_manufacturer_data.second.data(), cached_manufacturer_data.second.size());

    return SIMPLEBLE_SUCCESS;
}

simpleble_err_t simpleble_peripheral_read(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                          simpleble_uuid_t characteristic, uint8_t** data, size_t* data_length) {
    if (handle == nullptr || data == nullptr || data_length == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    std::optional<SimpleBLE::ByteArray> read_data = peripheral->read(SimpleBLE::BluetoothUUID(service.value),
                                                                     SimpleBLE::BluetoothUUID(characteristic.value));

    if (!read_data.has_value()) {
        return SIMPLEBLE_FAILURE;
    }

    // Create a combined UUID between the service and characteristic UUIDs
    std::string combined_uuid = std::string(service.value) + ":" + std::string(characteristic.value);
    cache_read_data[handle][combined_uuid] = read_data.value();

    // We'll return pointers to the cached data as a way to provide some degree of memory safety
    *data = (uint8_t*)cache_read_data[handle][combined_uuid].data();
    *data_length = cache_read_data[handle][combined_uuid].size();

    return SIMPLEBLE_SUCCESS;
}

simpleble_err_t simpleble_peripheral_write_request(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                   simpleble_uuid_t characteristic, uint8_t* data, size_t data_length) {
    if (handle == nullptr || data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->write_request(SimpleBLE::BluetoothUUID(service.value),
                                             SimpleBLE::BluetoothUUID(characteristic.value),
                                             SimpleBLE::ByteArray((const char*)data, data_length));

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_write_command(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                   simpleble_uuid_t characteristic, uint8_t* data, size_t data_length) {
    if (handle == nullptr || data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->write_command(SimpleBLE::BluetoothUUID(service.value),
                                             SimpleBLE::BluetoothUUID(characteristic.value),
                                             SimpleBLE::ByteArray((const char*)data, data_length));

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_notify(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                            simpleble_uuid_t characteristic,
                                            void (*callback)(simpleble_uuid_t service, simpleble_uuid_t characteristic,
                                                             uint8_t* data, size_t data_length)) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->notify(
        SimpleBLE::BluetoothUUID(service.value), SimpleBLE::BluetoothUUID(characteristic.value),
        [=](SimpleBLE::ByteArray data) { callback(service, characteristic, (uint8_t*)data.data(), data.size()); });

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_indicate(
    simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic,
    void (*callback)(simpleble_uuid_t service, simpleble_uuid_t characteristic, uint8_t* data, size_t data_length)) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->indicate(
        SimpleBLE::BluetoothUUID(service.value), SimpleBLE::BluetoothUUID(characteristic.value),
        [=](SimpleBLE::ByteArray data) { callback(service, characteristic, (uint8_t*)data.data(), data.size()); });

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_unsubscribe(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                 simpleble_uuid_t characteristic) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->unsubscribe(SimpleBLE::BluetoothUUID(service.value),
                                           SimpleBLE::BluetoothUUID(characteristic.value));

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_set_callback_on_connected(simpleble_peripheral_t handle,
                                                               void (*callback)(simpleble_peripheral_t peripheral)) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->set_callback_on_connected([=]() { callback(handle); });

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_set_callback_on_disconnected(simpleble_peripheral_t handle,
                                                                  void (*callback)(simpleble_peripheral_t peripheral)) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->set_callback_on_disconnected([=]() { callback(handle); });

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}
