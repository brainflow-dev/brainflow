#include <simplecble/peripheral.h>

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>

#include <climits>
#include <cstring>
#include <map>

void simpleble_peripheral_release_handle(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    delete peripheral;
}

void* simpleble_peripheral_underlying(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        return peripheral->underlying();
    } catch (...) {
        return nullptr;
    }
}

char* simpleble_peripheral_identifier(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        std::string identifier = peripheral->identifier();
        char* c_identifier = (char*)malloc(identifier.size() + 1);
        strcpy(c_identifier, identifier.c_str());
        return c_identifier;
    } catch (...) {
        return nullptr;
    }
}

char* simpleble_peripheral_address(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        std::string address = peripheral->address();
        char* c_address = (char*)malloc(address.size() + 1);
        strcpy(c_address, address.c_str());
        return c_address;
    } catch (...) {
        return nullptr;
    }
}

simpleble_address_type_t simpleble_peripheral_address_type(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_ADDRESS_TYPE_UNSPECIFIED;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        SimpleBLE::BluetoothAddressType address_type = peripheral->address_type();
        return (simpleble_address_type_t)address_type;
    } catch (...) {
        return SIMPLEBLE_ADDRESS_TYPE_UNSPECIFIED;
    }
}

int16_t simpleble_peripheral_rssi(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return INT16_MIN;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        return peripheral->rssi();
    } catch (...) {
        return INT16_MIN;
    }
}

int16_t simpleble_peripheral_tx_power(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return INT16_MIN;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        return peripheral->tx_power();
    } catch (...) {
        return INT16_MIN;
    }
}

uint16_t simpleble_peripheral_mtu(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        return peripheral->mtu();
    } catch (...) {
        return 0;
    }
}

simpleble_err_t simpleble_peripheral_connect(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->connect();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_disconnect(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->disconnect();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_is_connected(simpleble_peripheral_t handle, bool* connected) {
    if (handle == nullptr || connected == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        *connected = peripheral->is_connected();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_is_connectable(simpleble_peripheral_t handle, bool* connectable) {
    if (handle == nullptr || connectable == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        *connectable = peripheral->is_connectable();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_is_paired(simpleble_peripheral_t handle, bool* paired) {
    if (handle == nullptr || paired == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        *paired = peripheral->is_paired();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_unpair(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->unpair();
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

size_t simpleble_peripheral_services_count(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        return peripheral->services().size();
    } catch (...) {
        return 0;
    }
}

simpleble_err_t simpleble_peripheral_services_get(simpleble_peripheral_t handle, size_t index,
                                                  simpleble_service_t* services) {
    if (handle == nullptr || services == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    memset(services, 0, sizeof(simpleble_service_t));

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        auto peripheral_services = peripheral->services();

        if (index >= peripheral_services.size()) {
            return SIMPLEBLE_FAILURE;
        }

        SimpleBLE::Service service = peripheral_services[index];

        memcpy(services->uuid.value, service.uuid().c_str(), SIMPLEBLE_UUID_STR_LEN);

        services->data_length = service.data().size();
        memcpy(services->data, service.data().data(), service.data().size());

        services->characteristic_count = service.characteristics().size();
        if (services->characteristic_count > SIMPLEBLE_CHARACTERISTIC_MAX_COUNT) {
            services->characteristic_count = SIMPLEBLE_CHARACTERISTIC_MAX_COUNT;
        }

        for (size_t i = 0; i < services->characteristic_count; i++) {
            SimpleBLE::Characteristic characteristic = service.characteristics()[i];

            services->characteristics[i].can_read = characteristic.can_read();
            services->characteristics[i].can_write_request = characteristic.can_write_request();
            services->characteristics[i].can_write_command = characteristic.can_write_command();
            services->characteristics[i].can_notify = characteristic.can_notify();
            services->characteristics[i].can_indicate = characteristic.can_indicate();

            memcpy(services->characteristics[i].uuid.value, characteristic.uuid().c_str(), SIMPLEBLE_UUID_STR_LEN);
            services->characteristics[i].descriptor_count = characteristic.descriptors().size();

            if (services->characteristics[i].descriptor_count > SIMPLEBLE_DESCRIPTOR_MAX_COUNT) {
                services->characteristics[i].descriptor_count = SIMPLEBLE_DESCRIPTOR_MAX_COUNT;
            }

            for (size_t j = 0; j < services->characteristics[i].descriptor_count; j++) {
                SimpleBLE::Descriptor descriptor = characteristic.descriptors()[j];

                memcpy(services->characteristics[i].descriptors[j].uuid.value, descriptor.uuid().c_str(),
                       SIMPLEBLE_UUID_STR_LEN);
            }
        }

        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

size_t simpleble_peripheral_manufacturer_data_count(simpleble_peripheral_t handle) {
    if (handle == nullptr) {
        return 0;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        return peripheral->manufacturer_data().size();
    } catch (...) {
        return 0;
    }
}

simpleble_err_t simpleble_peripheral_manufacturer_data_get(simpleble_peripheral_t handle, size_t index,
                                                           simpleble_manufacturer_data_t* manufacturer_data) {
    if (handle == nullptr || manufacturer_data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        auto peripheral_manufacturer_data = peripheral->manufacturer_data();

        if (index >= peripheral_manufacturer_data.size()) {
            return SIMPLEBLE_FAILURE;
        }

        // Build an iterator and advance to the expected element
        std::map<uint16_t, SimpleBLE::ByteArray>::iterator it = peripheral_manufacturer_data.begin();
        for (size_t i = 0; i < index; i++) {
            it++;
        }

        auto& selected_manufacturer_data = *it;
        manufacturer_data->manufacturer_id = selected_manufacturer_data.first;
        manufacturer_data->data_length = selected_manufacturer_data.second.size();
        memcpy(manufacturer_data->data, selected_manufacturer_data.second.data(),
               selected_manufacturer_data.second.size());

        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
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
    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        SimpleBLE::ByteArray read_data = peripheral->read(SimpleBLE::BluetoothUUID(service.value),
                                                          SimpleBLE::BluetoothUUID(characteristic.value));

        *data_length = read_data.size();
        *data = static_cast<uint8_t*>(malloc(*data_length));
        memcpy(*data, read_data.data(), *data_length);

        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_write_request(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                   simpleble_uuid_t characteristic, const uint8_t* data,
                                                   size_t data_length) {
    if (handle == nullptr || data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->write_request(SimpleBLE::BluetoothUUID(service.value),
                                  SimpleBLE::BluetoothUUID(characteristic.value),
                                  SimpleBLE::ByteArray((const char*)data, data_length));
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_write_command(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                   simpleble_uuid_t characteristic, const uint8_t* data,
                                                   size_t data_length) {
    if (handle == nullptr || data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->write_command(SimpleBLE::BluetoothUUID(service.value),
                                  SimpleBLE::BluetoothUUID(characteristic.value),
                                  SimpleBLE::ByteArray((const char*)data, data_length));
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_notify(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                            simpleble_uuid_t characteristic,
                                            void (*callback)(simpleble_peripheral_t, simpleble_uuid_t, simpleble_uuid_t,
                                                             const uint8_t*, size_t, void*),
                                            void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->notify(SimpleBLE::BluetoothUUID(service.value), SimpleBLE::BluetoothUUID(characteristic.value),
                           [=](SimpleBLE::ByteArray data) {
                               callback(handle, service, characteristic, (const uint8_t*)data.data(), data.size(),
                                        userdata);
                           });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_indicate(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                              simpleble_uuid_t characteristic,
                                              void (*callback)(simpleble_peripheral_t, simpleble_uuid_t,
                                                               simpleble_uuid_t, const uint8_t*, size_t, void*),
                                              void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->indicate(SimpleBLE::BluetoothUUID(service.value), SimpleBLE::BluetoothUUID(characteristic.value),
                             [=](SimpleBLE::ByteArray data) {
                                 callback(handle, service, characteristic, (const uint8_t*)data.data(), data.size(),
                                          userdata);
                             });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_unsubscribe(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                 simpleble_uuid_t characteristic) {
    if (handle == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->unsubscribe(SimpleBLE::BluetoothUUID(service.value),
                                SimpleBLE::BluetoothUUID(characteristic.value));
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_read_descriptor(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                     simpleble_uuid_t characteristic, simpleble_uuid_t descriptor,
                                                     uint8_t** data, size_t* data_length) {
    if (handle == nullptr || data == nullptr || data_length == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    // Clear the initial values for safety
    *data = nullptr;
    *data_length = 0;

    // Perform the read operation
    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        SimpleBLE::ByteArray read_data =
            peripheral->read(SimpleBLE::BluetoothUUID(service.value), SimpleBLE::BluetoothUUID(characteristic.value),
                             SimpleBLE::BluetoothUUID(descriptor.value));

        *data_length = read_data.size();
        *data = static_cast<uint8_t*>(malloc(*data_length));
        memcpy(*data, read_data.data(), *data_length);

        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_write_descriptor(simpleble_peripheral_t handle, simpleble_uuid_t service,
                                                      simpleble_uuid_t characteristic, simpleble_uuid_t descriptor,
                                                      const uint8_t* data, size_t data_length) {
    if (handle == nullptr || data == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->write(SimpleBLE::BluetoothUUID(service.value), SimpleBLE::BluetoothUUID(characteristic.value),
                          SimpleBLE::BluetoothUUID(descriptor.value),
                          SimpleBLE::ByteArray((const char*)data, data_length));
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_set_callback_on_connected(simpleble_peripheral_t handle,
                                                               void (*callback)(simpleble_peripheral_t, void*),
                                                               void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->set_callback_on_connected([=]() { callback(handle, userdata); });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}

simpleble_err_t simpleble_peripheral_set_callback_on_disconnected(simpleble_peripheral_t handle,
                                                                  void (*callback)(simpleble_peripheral_t, void*),
                                                                  void* userdata) {
    if (handle == nullptr || callback == nullptr) {
        return SIMPLEBLE_FAILURE;
    }

    SimpleBLE::Peripheral* peripheral = (SimpleBLE::Peripheral*)handle;
    try {
        peripheral->set_callback_on_disconnected([=]() { callback(handle, userdata); });
        return SIMPLEBLE_SUCCESS;
    } catch (...) {
        return SIMPLEBLE_FAILURE;
    }
}