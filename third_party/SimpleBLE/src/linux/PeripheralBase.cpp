#include "PeripheralBase.h"

#include <simpleble/Exceptions.h>
#include "Bluez.h"

#include <iostream>

using namespace SimpleBLE;
using namespace std::chrono_literals;

PeripheralBase::PeripheralBase(std::shared_ptr<BluezDevice> device) {
    device_ = device;

    // NOTE: As Bluez will delete the object if it was advertising as non-connectable,
    // we need to keep a copy of some of the properties around.
    identifier_ = device->get_name();
    address_ = device->get_address();
}

PeripheralBase::~PeripheralBase() {}

std::string PeripheralBase::identifier() { return identifier_; }

BluetoothAddress PeripheralBase::address() { return address_; }

void PeripheralBase::connect() {
    // Attempt to connect to the device.
    for (size_t i = 0; i < 5; i++) {
        if (_attempt_connect()) {
            break;
        }
    }

    if (!is_connected()) {
        throw Exception::OperationFailed();
    }
}

void PeripheralBase::disconnect() {
    // Attempt to connect to the device.
    for (size_t i = 0; i < 5; i++) {
        if (_attempt_disconnect()) {
            break;
        }
    }

    if (is_connected()) {
        throw Exception::OperationFailed();
    }
}

bool PeripheralBase::is_connected() {
    auto device = _get_device();

    // NOTE: For Bluez, a device being connected means that it's both
    // connected and services have been resolved.
    return device->Property_Connected() && device->Property_ServicesResolved();
}

bool PeripheralBase::is_connectable() { return identifier_ != ""; }

std::vector<BluetoothService> PeripheralBase::services() {
    auto device = _get_device();

    std::vector<BluetoothService> service_list;
    for (auto service_uuid : device->get_service_list()) {
        BluetoothService service;
        service.uuid = service_uuid;
        service.characteristics = device->get_characteristic_list(service_uuid);
        service_list.push_back(service);
    }

    return service_list;
}

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() {
    auto device = _get_device();

    std::map<uint16_t, ByteArray> manufacturer_data;
    for (auto& [manufacturer_id, value_array] : device->get_manufacturer_data()) {
        manufacturer_data[manufacturer_id] = ByteArray((const char*)value_array.data(), value_array.size());
    }

    return manufacturer_data;
}

ByteArray PeripheralBase::read(BluetoothUUID service, BluetoothUUID characteristic) {
    // TODO: Check if the characteristic is readable.
    auto characteristic_object = _get_characteristic(service, characteristic);

    std::vector<uint8_t> value = characteristic_object->Property_Value();
    return ByteArray((const char*)value.data(), value.size());
}

void PeripheralBase::write_request(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data) {
    // TODO: Check if the characteristic is writable.
    auto characteristic_object = _get_characteristic(service, characteristic);
    characteristic_object->write_request((const uint8_t*)data.c_str(), data.size());
}

void PeripheralBase::write_command(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data) {
    // TODO: Check if the characteristic is writable.
    auto characteristic_object = _get_characteristic(service, characteristic);
    characteristic_object->write_command((const uint8_t*)data.c_str(), data.size());
}

void PeripheralBase::notify(BluetoothUUID service, BluetoothUUID characteristic,
                            std::function<void(ByteArray payload)> callback) {
    // TODO: What to do if the characteristic is already being notified?
    // TODO: Check if the property can be notified.
    auto characteristic_object = _get_characteristic(service, characteristic);
    characteristic_object->ValueChanged = [=](std::vector<uint8_t> value) {
        callback(ByteArray((const char*)value.data(), value.size()));
    };
    characteristic_object->Action_StartNotify();
}

void PeripheralBase::indicate(BluetoothUUID service, BluetoothUUID characteristic,
                              std::function<void(ByteArray payload)> callback) {
    // TODO: What to do if the characteristic is already being indicated?
    // TODO: Check if the property can be indicated.
    auto characteristic_object = _get_characteristic(service, characteristic);
    characteristic_object->ValueChanged = [=](std::vector<uint8_t> value) {
        callback(ByteArray((const char*)value.data(), value.size()));
    };
    characteristic_object->Action_StartNotify();
}

void PeripheralBase::unsubscribe(BluetoothUUID service, BluetoothUUID characteristic) {
    // TODO: What to do if the characteristic is not being notified?
    auto characteristic_object = _get_characteristic(service, characteristic);
    characteristic_object->Action_StopNotify();

    // Wait for the characteristic to stop notifying.
    // TODO: Upgrade SimpleDBus to provide a way to wait for this signal.
    auto timeout = std::chrono::system_clock::now() + 5s;
    while (characteristic_object->Property_Notifying() && std::chrono::system_clock::now() < timeout) {
        std::this_thread::sleep_for(50ms);
    }
}

void PeripheralBase::set_callback_on_connected(std::function<void()> on_connected) {
    callback_on_connected_ = on_connected;
}

void PeripheralBase::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    callback_on_disconnected_ = on_disconnected;
}

// Private methods

bool PeripheralBase::_attempt_connect() {
    auto device = _get_device();

    // Set the OnServiceDiscovered callback, which should properly indicate
    // if the connection has been established.
    device->OnServicesResolved = [=]() { this->connection_cv_.notify_all(); };

    // Set the OnDisconnected callback
    device->OnDisconnected = [this]() {
        this->disconnection_cv_.notify_all();

        if (this->callback_on_disconnected_) {
            this->callback_on_disconnected_();
        }
    };

    device->Action_Connect();

    // Wait for the connection to be confirmed.
    // The condition variable will return false if the connection was not established.
    std::unique_lock<std::mutex> lock(connection_mutex_);
    return connection_cv_.wait_for(lock, 1s, [this]() { return is_connected(); });
}

bool PeripheralBase::_attempt_disconnect() {
    auto device = _get_device();

    device->Action_Disconnect();

    // Wait for the disconnection to be confirmed.
    // The condition variable will return false if the connection is still active.
    std::unique_lock<std::mutex> lock(disconnection_mutex_);
    return disconnection_cv_.wait_for(lock, 1s, [this]() { return !is_connected(); });
}

std::shared_ptr<BluezDevice> PeripheralBase::_get_device() {
    std::shared_ptr<BluezDevice> device = device_.lock();

    if (!device) {
        throw Exception::InvalidReference();
    }

    return device;
}

std::shared_ptr<BluezGattCharacteristic> PeripheralBase::_get_characteristic(BluetoothUUID service_uuid,
                                                                             BluetoothUUID characteristic_uuid) {
    auto device = _get_device();

    auto service = device->get_service(service_uuid);
    if (!service) {
        throw Exception::ServiceNotFound(service_uuid);
    }

    auto characteristic = service->get_characteristic(characteristic_uuid);
    if (!characteristic) {
        throw Exception::CharacteristicNotFound(characteristic_uuid);
    }

    return characteristic;
}
