#include "PeripheralBase.h"

#include "CharacteristicBuilder.h"
#include "DescriptorBuilder.h"
#include "ServiceBuilder.h"

#include <simpleble/Exceptions.h>
#include <algorithm>
#include "CommonUtils.h"
#include "LoggingInternal.h"

using namespace SimpleBLE;
using namespace std::chrono_literals;

PeripheralBase::PeripheralBase(Android::ScanResult scan_result) : _device(scan_result.getDevice()) {
    _btGattCallback.set_callback_onConnectionStateChange([this](bool connected) {
        // If a connection has been established, request service discovery.
        if (connected) {
            _gatt.discoverServices();
        } else {
            // TODO: Whatever cleanup is necessary when disconnected.
            SIMPLEBLE_LOG_INFO("Disconnected from device");
            SAFE_CALLBACK_CALL(callback_on_disconnected_);
        }
    });

    _btGattCallback.set_callback_onServicesDiscovered([this]() {
        _services = _gatt.getServices();

        // Notify the user that the connection has been established once services have been discovered.
        SAFE_CALLBACK_CALL(callback_on_connected_);
    });
}

PeripheralBase::~PeripheralBase() {}

void PeripheralBase::update_advertising_data(Android::ScanResult scan_result) {}

void* PeripheralBase::underlying() const { return nullptr; }

std::string PeripheralBase::identifier() { return _device.getName(); }

BluetoothAddress PeripheralBase::address() { return BluetoothAddress(_device.getAddress()); }

BluetoothAddressType PeripheralBase::address_type() { return BluetoothAddressType::UNSPECIFIED; }

int16_t PeripheralBase::rssi() { return 0; }

int16_t PeripheralBase::tx_power() { return 0; }

uint16_t PeripheralBase::mtu() { return _btGattCallback.mtu; }

void PeripheralBase::connect() { _gatt = _device.connectGatt(false, _btGattCallback); }

void PeripheralBase::disconnect() { _gatt.disconnect(); }

bool PeripheralBase::is_connected() { return _btGattCallback.connected && _btGattCallback.services_discovered; }

bool PeripheralBase::is_connectable() { return false; }

bool PeripheralBase::is_paired() { return false; }

void PeripheralBase::unpair() {}

std::vector<Service> PeripheralBase::services() {
    std::vector<Service> service_list;
    for (auto service : _services) {
        // Build the list of characteristics for the service.
        std::vector<Characteristic> characteristic_list;
        for (auto characteristic : service.getCharacteristics()) {
            // Build the list of descriptors for the characteristic.
            std::vector<Descriptor> descriptor_list;
            for (auto descriptor : characteristic.getDescriptors()) {
                descriptor_list.push_back(DescriptorBuilder(descriptor.getUuid()));
            }

            int flags = characteristic.getProperties();

            bool can_read = flags & Android::BluetoothGattCharacteristic::PROPERTY_READ;
            bool can_write_request = flags & Android::BluetoothGattCharacteristic::PROPERTY_WRITE;
            bool can_write_command = flags & Android::BluetoothGattCharacteristic::PROPERTY_WRITE_NO_RESPONSE;
            bool can_notify = flags & Android::BluetoothGattCharacteristic::PROPERTY_NOTIFY;
            bool can_indicate = flags & Android::BluetoothGattCharacteristic::PROPERTY_INDICATE;

            characteristic_list.push_back(CharacteristicBuilder(characteristic.getUuid(), descriptor_list, can_read,
                                                                can_write_request, can_write_command, can_notify,
                                                                can_indicate));
        }

        service_list.push_back(ServiceBuilder(service.getUuid(), characteristic_list));
    }

    return service_list;
}

std::vector<Service> PeripheralBase::advertised_services() { return std::vector<Service>(); }

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() { return std::map<uint16_t, ByteArray>(); }

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    SIMPLEBLE_LOG_INFO("Reading characteristic " + characteristic);

    auto characteristic_obj = _fetch_characteristic(service, characteristic);

    _btGattCallback.set_flag_characteristicReadPending(characteristic_obj.getObject());
    if (!_gatt.readCharacteristic(characteristic_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to read characteristic " + characteristic);
    }

    auto value = _btGattCallback.wait_flag_characteristicReadPending(characteristic_obj.getObject());
    return ByteArray(value);
}

void PeripheralBase::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                   ByteArray const& data) {
    SIMPLEBLE_LOG_INFO("Writing request to characteristic " + characteristic);

    auto characteristic_obj = _fetch_characteristic(service, characteristic);

    characteristic_obj.setWriteType(Android::BluetoothGattCharacteristic::WRITE_TYPE_DEFAULT);
    characteristic_obj.setValue(std::vector<uint8_t>(data.begin(), data.end()));

    _btGattCallback.set_flag_characteristicWritePending(characteristic_obj.getObject());
    if (!_gatt.writeCharacteristic(characteristic_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to write characteristic " + characteristic);
    }
    _btGattCallback.wait_flag_characteristicWritePending(characteristic_obj.getObject());
}

void PeripheralBase::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                   ByteArray const& data) {
    SIMPLEBLE_LOG_INFO("Writing command to characteristic " + characteristic);

    auto characteristic_obj = _fetch_characteristic(service, characteristic);

    characteristic_obj.setWriteType(Android::BluetoothGattCharacteristic::WRITE_TYPE_NO_RESPONSE);
    characteristic_obj.setValue(std::vector<uint8_t>(data.begin(), data.end()));

    _btGattCallback.set_flag_characteristicWritePending(characteristic_obj.getObject());
    if (!_gatt.writeCharacteristic(characteristic_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to write characteristic " + characteristic);
    }
    _btGattCallback.wait_flag_characteristicWritePending(characteristic_obj.getObject());
}

void PeripheralBase::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                            std::function<void(ByteArray payload)> callback) {
    SIMPLEBLE_LOG_INFO("Subscribing to characteristic " + characteristic);

    auto characteristic_obj = _fetch_characteristic(service, characteristic);
    auto descriptor_obj = _fetch_descriptor(service, characteristic,
                                            Android::BluetoothGattDescriptor::CLIENT_CHARACTERISTIC_CONFIG);

    _btGattCallback.set_callback_onCharacteristicChanged(characteristic_obj.getObject(),
                                                         [callback](std::vector<uint8_t> data) {
                                                             ByteArray payload(data);
                                                             callback(payload);
                                                         });
    bool success = _gatt.setCharacteristicNotification(characteristic_obj, true);
    if (!success) {
        throw SimpleBLE::Exception::OperationFailed("Failed to subscribe to characteristic " + characteristic);
    }

    _btGattCallback.set_flag_descriptorWritePending(descriptor_obj.getObject().get());
    descriptor_obj.setValue(Android::BluetoothGattDescriptor::ENABLE_NOTIFICATION_VALUE);
    if (!_gatt.writeDescriptor(descriptor_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to write descriptor for characteristic " + characteristic);
    }
    _btGattCallback.wait_flag_descriptorWritePending(descriptor_obj.getObject().get());
}

void PeripheralBase::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                              std::function<void(ByteArray payload)> callback) {
    SIMPLEBLE_LOG_INFO("Subscribing to characteristic " + characteristic);

    auto characteristic_obj = _fetch_characteristic(service, characteristic);
    auto descriptor_obj = _fetch_descriptor(service, characteristic,
                                            Android::BluetoothGattDescriptor::CLIENT_CHARACTERISTIC_CONFIG);

    _btGattCallback.set_callback_onCharacteristicChanged(characteristic_obj.getObject(),
                                                         [callback](std::vector<uint8_t> data) {
                                                             ByteArray payload(data);
                                                             callback(payload);
                                                         });
    bool success = _gatt.setCharacteristicNotification(characteristic_obj, true);
    if (!success) {
        throw SimpleBLE::Exception::OperationFailed("Failed to subscribe to characteristic " + characteristic);
    }

    _btGattCallback.set_flag_descriptorWritePending(descriptor_obj.getObject().get());
    descriptor_obj.setValue(Android::BluetoothGattDescriptor::ENABLE_INDICATION_VALUE);
    if (!_gatt.writeDescriptor(descriptor_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to write descriptor for characteristic " + characteristic);
    }
    _btGattCallback.wait_flag_descriptorWritePending(descriptor_obj.getObject().get());
}

void PeripheralBase::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    SIMPLEBLE_LOG_INFO("Unsubscribing from characteristic " + characteristic);

    auto characteristic_obj = _fetch_characteristic(service, characteristic);
    auto descriptor_obj = _fetch_descriptor(service, characteristic,
                                            Android::BluetoothGattDescriptor::CLIENT_CHARACTERISTIC_CONFIG);

    _btGattCallback.set_flag_descriptorWritePending(descriptor_obj.getObject().get());
    descriptor_obj.setValue(Android::BluetoothGattDescriptor::DISABLE_NOTIFICATION_VALUE);
    if (!_gatt.writeDescriptor(descriptor_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to write descriptor for characteristic " + characteristic);
    }
    _btGattCallback.wait_flag_descriptorWritePending(descriptor_obj.getObject().get());

    _btGattCallback.clear_callback_onCharacteristicChanged(characteristic_obj.getObject());
    bool success = _gatt.setCharacteristicNotification(characteristic_obj, true);
    if (!success) {
        throw SimpleBLE::Exception::OperationFailed("Failed to subscribe to characteristic " + characteristic);
    }
}

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               BluetoothUUID const& descriptor) {
    SIMPLEBLE_LOG_INFO("Reading descriptor " + descriptor);

    auto descriptor_obj = _fetch_descriptor(service, characteristic, descriptor);

    _btGattCallback.set_flag_descriptorReadPending(descriptor_obj.getObject().get());
    if (!_gatt.readDescriptor(descriptor_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to read descriptor " + descriptor);
    }

    auto value = _btGattCallback.wait_flag_descriptorReadPending(descriptor_obj.getObject().get());
    return ByteArray(value);
}

void PeripheralBase::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                           BluetoothUUID const& descriptor, ByteArray const& data) {
    SIMPLEBLE_LOG_INFO("Writing descriptor " + descriptor);

    auto descriptor_obj = _fetch_descriptor(service, characteristic, descriptor);

    _btGattCallback.set_flag_descriptorWritePending(descriptor_obj.getObject().get());
    descriptor_obj.setValue(std::vector<uint8_t>(data.begin(), data.end()));
    if (!_gatt.writeDescriptor(descriptor_obj)) {
        throw SimpleBLE::Exception::OperationFailed("Failed to write descriptor " + descriptor);
    }
    _btGattCallback.wait_flag_descriptorWritePending(descriptor_obj.getObject().get());
}

void PeripheralBase::set_callback_on_connected(std::function<void()> on_connected) {
    if (on_connected) {
        callback_on_connected_.load(std::move(on_connected));
    } else {
        callback_on_connected_.unload();
    }
}

void PeripheralBase::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    if (on_disconnected) {
        callback_on_disconnected_.load(std::move(on_disconnected));
    } else {
        callback_on_disconnected_.unload();
    }
}

// NOTE: This approach to retrieve the characteristic and descriptor objects is not ideal, as it involves
//       iterating on Java objects and returning a copy of the desired object. This will be improved in the future
//       if performance becomes a bottleneck.
Android::BluetoothGattCharacteristic PeripheralBase::_fetch_characteristic(const BluetoothUUID& service_uuid,
                                                                           const BluetoothUUID& characteristic_uuid) {
    for (auto& service : _services) {
        if (service.getUuid() == service_uuid) {
            for (auto& characteristic : service.getCharacteristics()) {
                if (characteristic.getUuid() == characteristic_uuid) {
                    return characteristic;
                }
                throw SimpleBLE::Exception::CharacteristicNotFound(characteristic_uuid);
            }
        }
    }

    throw SimpleBLE::Exception::ServiceNotFound(service_uuid);
}

Android::BluetoothGattDescriptor PeripheralBase::_fetch_descriptor(const BluetoothUUID& service_uuid,
                                                                   const BluetoothUUID& characteristic_uuid,
                                                                   const BluetoothUUID& descriptor_uuid) {
    for (auto& service : _services) {
        if (service.getUuid() == service_uuid) {
            for (auto& characteristic : service.getCharacteristics()) {
                if (characteristic.getUuid() == characteristic_uuid) {
                    for (auto& descriptor : characteristic.getDescriptors()) {
                        if (descriptor.getUuid() == descriptor_uuid) {
                            return descriptor;
                        }
                        throw SimpleBLE::Exception::DescriptorNotFound(descriptor_uuid);
                    }
                }
                throw SimpleBLE::Exception::CharacteristicNotFound(characteristic_uuid);
            }
        }
    }
    throw SimpleBLE::Exception::ServiceNotFound(service_uuid);
}