#include <simpleble_c/peripheral.h>

#include <simpleble/PeripheralSafe.h>
#include "PeripheralBase.h"
#include "PeripheralBuilder.h"

#include <cstring>
#include <map>

void simpleble_peripheral_release_handle(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    delete peripheral;
}

char* simpleble_peripheral_identifier(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    std::string identifier = peripheral->identifier().value_or("");
    char* c_identifier = (char*)malloc(identifier.size() + 1);
    strcpy(c_identifier, identifier.c_str());
    return c_identifier;
}

char* simpleble_peripheral_address(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    std::string address = peripheral->address().value_or("");
    char* c_address = (char*)malloc(address.size() + 1);
    strcpy(c_address, address.c_str());
    return c_address;
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

size_t simpleble_peripheral_services_count(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    auto services = peripheral->services();
    if (services.has_value()) {
        return services.value().size();
    } else {
        return 0;
    }
}

simpleble_err_t simpleble_peripheral_services_get(simpleble_peripheral_t handle, size_t index,
                                                  simpleble_service_t* services) {
    if (handle == nullptr || services == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    auto peripheral_services = peripheral->services();
    if (!peripheral_services.has_value()) {
        return SIMPLEBLE_FAILURE;
    }

    if (index >= peripheral_services.value().size()) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::BluetoothService& service = peripheral_services.value()[index];

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

size_t simpleble_peripheral_manufacturer_data_count(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    auto manufacturer_data = peripheral->manufacturer_data();
    if (manufacturer_data.has_value()) {
        return manufacturer_data.value().size();
    } else {
        return 0;
    }
}

simpleble_err_t simpleble_peripheral_manufacturer_data_get(simpleble_peripheral_t handle, size_t index,
                                                           simpleble_manufacturer_data_t* manufacturer_data) {
    if (handle == nullptr || manufacturer_data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    auto peripheral_manufacturer_data = peripheral->manufacturer_data();
    if (!peripheral_manufacturer_data.has_value()) {
        return SIMPLEBLE_FAILURE;
    }

    if (index >= peripheral_manufacturer_data.value().size()) {
        return SIMPLEBLE_FAILURE;
    }

    // Build an iterator and advance to the expected element
    std::map<uint16_t, SimpleBLE::ByteArray>::iterator it = peripheral_manufacturer_data.value().begin();
    for (size_t i = 0; i < index; i++) {
        it++;
    }

    auto& selected_manufacturer_data = *it;
    manufacturer_data->manufacturer_id = selected_manufacturer_data.first;
    manufacturer_data->data_length = selected_manufacturer_data.second.size();
    memcpy(manufacturer_data->data, selected_manufacturer_data.second.data(), selected_manufacturer_data.second.size());

    return SIMPLEBLE_SUCCESS;
}

simpleble_err_t simpleble_peripheral_read(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                          simpleble_uuid_t characteristic, uint8_t** data, size_t* data_length) {
    if (handle == nullptr || data == nullptr || data_length == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    // Clear the initial values for safety
    *data = nullptr;
    *data_length = 0;

    // Perform the read operation
    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;
    std::optional<SimpleBLE::ByteArray> read_data = peripheral->read(SimpleBLE::BluetoothUUID(service.value),
                                                                     SimpleBLE::BluetoothUUID(characteristic.value));

    // Early return if the operation failed
    if (!read_data.has_value()) {
        return SIMPLEBLE_FAILURE;
    }

    *data_length = read_data.value().size();
    *data = static_cast<uint8_t*>(malloc(*data_length));
    memcpy(*data, read_data.value().c_str(), *data_length);

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

simpleble_err_t simpleble_peripheral_notify(
    simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic,
    void (*callback)(simpleble_uuid_t, simpleble_uuid_t, uint8_t*, size_t, void*), void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->notify(SimpleBLE::BluetoothUUID(service.value),
                                      SimpleBLE::BluetoothUUID(characteristic.value), [=](SimpleBLE::ByteArray data) {
                                          callback(service, characteristic, (uint8_t*)data.data(), data.size(),
                                                   userdata);
                                      });

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_indicate(
    simpleble_peripheral_t handle, simpleble_uuid_t service, simpleble_uuid_t characteristic,
    void (*callback)(simpleble_uuid_t, simpleble_uuid_t, uint8_t*, size_t, void*), void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->indicate(SimpleBLE::BluetoothUUID(service.value),
                                        SimpleBLE::BluetoothUUID(characteristic.value), [=](SimpleBLE::ByteArray data) {
                                            callback(service, characteristic, (uint8_t*)data.data(), data.size(),
                                                     userdata);
                                        });

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
                                                               void (*callback)(simpleble_peripheral_t, void*),
                                                               void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->set_callback_on_connected([=]() { callback(handle, userdata); });

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}

simpleble_err_t simpleble_peripheral_set_callback_on_disconnected(simpleble_peripheral_t handle,
                                                                  void (*callback)(simpleble_peripheral_t, void*),
                                                                  void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Safe::Peripheral* peripheral = (SimpleBLE::Safe::Peripheral*)handle;

    bool success = peripheral->set_callback_on_disconnected([=]() { callback(handle, userdata); });

    return success ? SIMPLEBLE_SUCCESS : SIMPLEBLE_FAILURE;
}
