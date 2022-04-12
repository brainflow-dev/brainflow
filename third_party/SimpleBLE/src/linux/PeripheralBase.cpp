#include "PeripheralBase.h"

#include <simpleble/Exceptions.h>
#include <simplebluez/Exceptions.h>
#include <algorithm>

#include "Bluez.h"

const SimpleBLE::BluetoothUUID BATTERY_SERVICE_UUID = "0000180f-0000-1000-8000-00805f9b34fb";
const SimpleBLE::BluetoothUUID BATTERY_CHARACTERISTIC_UUID = "00002a19-0000-1000-8000-00805f9b34fb";

using namespace SimpleBLE;
using namespace std::chrono_literals;

PeripheralBase::PeripheralBase(std::shared_ptr<SimpleBluez::Device> device) : device_(device) {}

PeripheralBase::~PeripheralBase() {
    // TODO: A more extensive cleanup process is probably needed.
    device_->clear_on_disconnected();
    device_->clear_on_services_resolved();
}

std::string PeripheralBase::identifier() { return device_->name(); }

BluetoothAddress PeripheralBase::address() { return device_->address(); }

void PeripheralBase::connect() {
    // Set the OnDisconnected callback
    device_->set_on_disconnected([this]() {
        this->disconnection_cv_.notify_all();

        if (this->callback_on_disconnected_) {
            this->callback_on_disconnected_();
        }
    });

    // Set the OnServicesResolved callback
    device_->set_on_services_resolved([this]() { this->connection_cv_.notify_all(); });

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
    // NOTE: For Bluez, a device being connected means that it's both
    // connected and services have been resolved.
    return device_->connected() && device_->services_resolved();
}

bool PeripheralBase::is_connectable() { return device_->name() != ""; }

std::vector<BluetoothService> PeripheralBase::services() {
    bool is_battery_service_available = false;

    std::vector<BluetoothService> service_list;
    for (auto bluez_service : device_->services()) {
        BluetoothService service;
        service.uuid = bluez_service->uuid();

        // Check if the service is the battery service.
        if (service.uuid == BATTERY_SERVICE_UUID) {
            is_battery_service_available = true;
        }

        for (auto bluez_characteristic : bluez_service->characteristics()) {
            service.characteristics.push_back(bluez_characteristic->uuid());
        }
        service_list.push_back(service);
    }

    // If the battery service is not available, and the device has the appropriate interface, add it.
    if (!is_battery_service_available && device_->has_battery_interface()) {
        // Emulate the battery service through the Battery1 interface.
        BluetoothService service;
        service.uuid = BATTERY_SERVICE_UUID;
        service.characteristics.push_back(BATTERY_CHARACTERISTIC_UUID);
        service_list.push_back(service);
    }

    return service_list;
}

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() {
    std::map<uint16_t, ByteArray> manufacturer_data;
    for (auto& [manufacturer_id, value_array] : device_->manufacturer_data()) {
        manufacturer_data[manufacturer_id] = ByteArray((const char*)value_array.data(), value_array.size());
    }

    return manufacturer_data;
}

ByteArray PeripheralBase::read(BluetoothUUID service, BluetoothUUID characteristic) {
    // Check if the user is attempting to read the battery service/characteristic and if so,
    //  emulate the battery service through the Battery1 interface if it's not available.
    if (service == BATTERY_SERVICE_UUID && characteristic == BATTERY_CHARACTERISTIC_UUID &&
        device_->has_battery_interface()) {
        // If this point is reached, the battery service needs to be emulated.
        uint8_t battery_percentage = device_->battery_percentage();
        return ByteArray(reinterpret_cast<char*>(&battery_percentage), 1);
    }

    // Otherwise, attempt to read the characteristic using default mechanisms
    return _get_characteristic(service, characteristic)->read();
}

void PeripheralBase::write_request(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data) {
    // TODO: Check if the characteristic is writable.
    _get_characteristic(service, characteristic)->write_request(data);
}

void PeripheralBase::write_command(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data) {
    // TODO: Check if the characteristic is writable.
    _get_characteristic(service, characteristic)->write_command(data);
}

void PeripheralBase::notify(BluetoothUUID service, BluetoothUUID characteristic,
                            std::function<void(ByteArray payload)> callback) {
    // Check if the user is attempting to notify the battery service/characteristic and if so,
    //  emulate the battery service through the Battery1 interface if it's not available.
    if (service == BATTERY_SERVICE_UUID && characteristic == BATTERY_CHARACTERISTIC_UUID &&
        device_->has_battery_interface()) {
        // If this point is reached, the battery service needs to be emulated.
        device_->set_on_battery_percentage_changed(
            [callback](uint8_t new_value) { callback(ByteArray(reinterpret_cast<char*>(&new_value), 1)); });
        return;
    }

    // Otherwise, attempt to read the characteristic using default mechanisms
    // TODO: What to do if the characteristic is already being notified?
    // TODO: Check if the property can be notified.
    auto characteristic_object = _get_characteristic(service, characteristic);
    characteristic_object->set_on_value_changed([callback](SimpleBluez::ByteArray new_value) { callback(new_value); });
    characteristic_object->start_notify();
}

void PeripheralBase::indicate(BluetoothUUID service, BluetoothUUID characteristic,
                              std::function<void(ByteArray payload)> callback) {
    notify(service, characteristic, callback);
}

void PeripheralBase::unsubscribe(BluetoothUUID service, BluetoothUUID characteristic) {
    // Check if the user is attempting to read the battery service/characteristic and if so,
    //  emulate the battery service through the Battery1 interface if it's not available.
    if (service == BATTERY_SERVICE_UUID && characteristic == BATTERY_CHARACTERISTIC_UUID &&
        device_->has_battery_interface()) {
        // If this point is reached, the battery service needs to be emulated.
        device_->clear_on_battery_percentage_changed();
        return;
    }

    // TODO: What to do if the characteristic is not being notified?
    auto characteristic_object = _get_characteristic(service, characteristic);
    characteristic_object->stop_notify();

    // Wait for the characteristic to stop notifying.
    // TODO: Upgrade SimpleDBus to provide a way to wait for this signal.
    auto timeout = std::chrono::system_clock::now() + 5s;
    while (characteristic_object->notifying() && std::chrono::system_clock::now() < timeout) {
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
    try {
        device_->connect();
    } catch (SimpleDBus::Exception::SendFailed& e) {
        return false;
    }

    // Wait for the connection to be confirmed.
    // The condition variable will return false if the connection was not established.
    std::unique_lock<std::mutex> lock(connection_mutex_);
    return connection_cv_.wait_for(lock, 2s, [this]() { return is_connected(); });
}

bool PeripheralBase::_attempt_disconnect() {
    device_->disconnect();

    // Wait for the disconnection to be confirmed.
    // The condition variable will return false if the connection is still active.
    std::unique_lock<std::mutex> lock(disconnection_mutex_);
    return disconnection_cv_.wait_for(lock, 1s, [this]() { return !is_connected(); });
}

std::shared_ptr<SimpleBluez::Characteristic> PeripheralBase::_get_characteristic(BluetoothUUID service_uuid,
                                                                                 BluetoothUUID characteristic_uuid) {
    try {
        return device_->get_characteristic(service_uuid, characteristic_uuid);
    } catch (SimpleBluez::Exception::ServiceNotFoundException& e) {
        throw Exception::ServiceNotFound(service_uuid);
    } catch (SimpleBluez::Exception::CharacteristicNotFoundException& e) {
        throw Exception::CharacteristicNotFound(characteristic_uuid);
    }
}
