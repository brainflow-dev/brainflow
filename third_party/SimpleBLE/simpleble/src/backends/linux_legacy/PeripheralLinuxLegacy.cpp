#include "PeripheralLinuxLegacy.h"

#include "BuildVec.h"
#include "BuilderBase.h"
#include "CharacteristicBase.h"
#include "DescriptorBase.h"
#include "ServiceBase.h"

#include <simpleble/Config.h>
#include <simpleble/Characteristic.h>
#include <simpleble/Descriptor.h>
#include <simpleble/Exceptions.h>
#include <simpleble/Service.h>
#include <simplebluezlegacy/Exceptions.h>
#include <algorithm>
#include <thread>
#include "CommonUtils.h"
#include "LoggingInternal.h"

const SimpleBLE::BluetoothUUID BATTERY_SERVICE_UUID = "0000180f-0000-1000-8000-00805f9b34fb";
const SimpleBLE::BluetoothUUID BATTERY_CHARACTERISTIC_UUID = "00002a19-0000-1000-8000-00805f9b34fb";

using namespace SimpleBLE;
using namespace std::chrono_literals;

PeripheralLinuxLegacy::PeripheralLinuxLegacy(std::shared_ptr<SimpleBluezLegacy::Device> device,
                                 std::shared_ptr<SimpleBluezLegacy::Adapter> adapter)
    : device_(std::move(device)), adapter_(std::move(adapter)) {}

PeripheralLinuxLegacy::~PeripheralLinuxLegacy() {
    // Clear the callbacks to prevent any further events from being sent to the user.
    this->callback_on_connected_.unload();
    this->callback_on_disconnected_.unload();

    device_->clear_on_disconnected();
    device_->clear_on_services_resolved();
    _cleanup_characteristics();
}

void* PeripheralLinuxLegacy::underlying() const { return device_.get(); }

std::string PeripheralLinuxLegacy::identifier() { return device_->name(); }

BluetoothAddress PeripheralLinuxLegacy::address() { return device_->address(); }

BluetoothAddressType PeripheralLinuxLegacy::address_type() {
    std::string address_type = device_->address_type();

    if (address_type == "public") {
        return BluetoothAddressType::PUBLIC;
    } else if (address_type == "public") {
        return BluetoothAddressType::RANDOM;
    } else {
        return BluetoothAddressType::UNSPECIFIED;
    }
}

int16_t PeripheralLinuxLegacy::rssi() { return device_->rssi(); }

int16_t PeripheralLinuxLegacy::tx_power() { return device_->tx_power(); }

uint16_t PeripheralLinuxLegacy::mtu() {
    if (!is_connected()) return 0;

    for (auto bluez_service : device_->services()) {
        for (auto bluez_characteristic : bluez_service->characteristics()) {
            // The value provided by Bluez includes an extra 3 bytes from the GATT header
            // which needs to be removed.
            return bluez_characteristic->mtu() - 3;
        }
    }
    return 0;
}

void PeripheralLinuxLegacy::connect() {
    device_->clear_on_disconnected();
    device_->set_on_services_resolved([this]() { this->connection_cv_.notify_all(); });

    // Attempt to connect to the device.
    for (size_t i = 0; i < 5; i++) {
        if (_attempt_connect()) {
            break;
        }
    }

    // Set the on_disconnected callback once the connection attempts are finished, thus
    // preventing disconnection events that should not be seen by the user.
    device_->set_on_disconnected([this]() {
        this->_cleanup_characteristics();
        this->disconnection_cv_.notify_all();

        SAFE_CALLBACK_CALL(this->callback_on_disconnected_);
    });

    if (!is_connected()) {
        throw Exception::OperationFailed();
    }

    SAFE_CALLBACK_CALL(this->callback_on_connected_);
}

void PeripheralLinuxLegacy::disconnect() {
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

bool PeripheralLinuxLegacy::is_connected() {
    // NOTE: For Bluez, a device being connected means that it's both
    // connected and services have been resolved.
    return device_->connected() && device_->services_resolved();
}

bool PeripheralLinuxLegacy::is_connectable() { return device_->name() != ""; }

bool PeripheralLinuxLegacy::is_paired() { return device_->paired(); }

void PeripheralLinuxLegacy::unpair() {
    if (device_->paired()) {
        adapter_->device_remove(device_->path());
    }
}

SharedPtrVector<ServiceBase> PeripheralLinuxLegacy::available_services() {
    bool is_battery_service_available = false;

    SharedPtrVector<ServiceBase> service_list;
    for (auto bluez_service : device_->services()) {
        // Check if the service is the battery service.
        if (bluez_service->uuid() == BATTERY_SERVICE_UUID) {
            is_battery_service_available = true;
        }

        // Build the list of characteristics for the service.
        SharedPtrVector<CharacteristicBase> characteristic_list;
        for (auto bluez_characteristic : bluez_service->characteristics()) {
            // Build the list of descriptors for the characteristic.
            SharedPtrVector<DescriptorBase> descriptor_list;
            for (auto bluez_descriptor : bluez_characteristic->descriptors()) {
                descriptor_list.push_back(std::make_shared<DescriptorBase>(bluez_descriptor->uuid()));
            }

            std::vector<std::string> flags = bluez_characteristic->flags();

            bool can_read = std::find(flags.begin(), flags.end(), "read") != flags.end();
            bool can_write_request = std::find(flags.begin(), flags.end(), "write") != flags.end();
            bool can_write_command = std::find(flags.begin(), flags.end(), "write-without-response") != flags.end();
            bool can_notify = std::find(flags.begin(), flags.end(), "notify") != flags.end();
            bool can_indicate = std::find(flags.begin(), flags.end(), "indicate") != flags.end();

            characteristic_list.push_back(
                std::make_shared<CharacteristicBase>(bluez_characteristic->uuid(), descriptor_list, can_read,
                                                     can_write_request, can_write_command, can_notify, can_indicate));
        }

        service_list.push_back(std::make_shared<ServiceBase>(bluez_service->uuid(), characteristic_list));
    }

    // If the battery service is not available, and the device has the appropriate interface, add it.
    if (!is_battery_service_available && device_->has_battery_interface()) {
        // Emulate the battery service through the Battery1 interface.
        SharedPtrVector<DescriptorBase> descriptor_list;
        SharedPtrVector<CharacteristicBase> characteristic_list = {std::make_shared<CharacteristicBase>(
            BATTERY_CHARACTERISTIC_UUID, descriptor_list, true, false, false, true, false)};
        service_list.push_back(std::make_shared<ServiceBase>(BATTERY_SERVICE_UUID, characteristic_list));
    }

    return service_list;
}

SharedPtrVector<ServiceBase> PeripheralLinuxLegacy::advertised_services() {
    SharedPtrVector<ServiceBase> service_list;
    for (auto& service_uuid : device_->uuids()) {
        service_list.push_back(std::make_shared<ServiceBase>(service_uuid));
    }

    return service_list;
}

std::map<uint16_t, ByteArray> PeripheralLinuxLegacy::manufacturer_data() { return device_->manufacturer_data(); }

ByteArray PeripheralLinuxLegacy::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
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

void PeripheralLinuxLegacy::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                    ByteArray const& data) {
    // TODO: Check if the characteristic is writable.
    // TODO: SimpleBluez::Characteristic::write_request() should also take ByteArray by const reference (but that's
    // another library)
    _get_characteristic(service, characteristic)->write_request(data);
}

void PeripheralLinuxLegacy::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                    ByteArray const& data) {
    // TODO: Check if the characteristic is writable.
    // TODO: SimpleBluez::Characteristic::write_command() should also take ByteArray by const reference (but that's
    // another library)
    _get_characteristic(service, characteristic)->write_command(data);
}

void PeripheralLinuxLegacy::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
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
    characteristic_object->set_on_value_changed([callback](SimpleBluezLegacy::ByteArray new_value) { callback(new_value); });
    characteristic_object->start_notify();
}

void PeripheralLinuxLegacy::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               std::function<void(ByteArray payload)> callback) {
    notify(service, characteristic, callback);
}

void PeripheralLinuxLegacy::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
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
    auto timeout = std::chrono::steady_clock::now() + 5s;
    while (characteristic_object->notifying() && std::chrono::steady_clock::now() < timeout) {
        std::this_thread::sleep_for(50ms);
    }
}

ByteArray PeripheralLinuxLegacy::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                BluetoothUUID const& descriptor) {
    return _get_descriptor(service, characteristic, descriptor)->read();
}

void PeripheralLinuxLegacy::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                            BluetoothUUID const& descriptor, ByteArray const& data) {
    _get_descriptor(service, characteristic, descriptor)->write(data);
}

void PeripheralLinuxLegacy::set_callback_on_connected(std::function<void()> on_connected) {
    if (on_connected) {
        callback_on_connected_.load(std::move(on_connected));
    } else {
        callback_on_connected_.unload();
    }
}

void PeripheralLinuxLegacy::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    if (on_disconnected) {
        callback_on_disconnected_.load(std::move(on_disconnected));
    } else {
        callback_on_disconnected_.unload();
    }
}

// Private methods

void PeripheralLinuxLegacy::_cleanup_characteristics() noexcept {
    // As this method can be called in multiple stages of a disconnection or object
    // destruction, the entire execution of this method is wrapped in a try-catch
    // block to prevent any exceptions from being thrown, as these will most certainly
    // crash the user application.
    try {
        // Clear all callbacks first to ensure that a failure during `stop_notify`
        // does not leave any dangling callbacks.
        if (device_->has_battery_interface()) {
            device_->clear_on_battery_percentage_changed();
        }

        for (auto bluez_service : device_->services()) {
            for (auto bluez_characteristic : bluez_service->characteristics()) {
                bluez_characteristic->clear_on_value_changed();
            }
        }

        // Stop notifying all characteristics.
        for (auto bluez_service : device_->services()) {
            for (auto bluez_characteristic : bluez_service->characteristics()) {
                try {
                    if (bluez_characteristic->notifying()) {
                        bluez_characteristic->stop_notify();
                    }
                } catch (std::exception const& e) {
                    SIMPLEBLE_LOG_WARN(fmt::format("Exception during characteristic cleanup: {}", e.what()));
                }
            }
        }
    } catch (std::exception const& e) {
        SIMPLEBLE_LOG_WARN(fmt::format("Exception during characteristic cleanup: {}", e.what()));
    } catch (...) {
        // It's possible during the cleanup process that the Bluez device has already
        // been removed, which could cause calls to cleanup methods to throw.
        SIMPLEBLE_LOG_WARN("Unknown exception during characteristic cleanup");
    }
}

bool PeripheralLinuxLegacy::_attempt_connect() {
    try {
        device_->connect();
    } catch (SimpleDBusLegacy::Exception::SendFailed const& e) {
        return false;
    }

    // Wait for the connection to be confirmed.
    // The condition variable will return false if the connection was not established.
    std::unique_lock<std::mutex> lock(connection_mutex_);
    return connection_cv_.wait_for(lock, Config::SimpleBluez::connection_timeout, [this]() { return is_connected(); });
}

bool PeripheralLinuxLegacy::_attempt_disconnect() {
    _cleanup_characteristics();

    device_->disconnect();

    // Wait for the disconnection to be confirmed.
    // The condition variable will return false if the connection is still active.
    std::unique_lock<std::mutex> lock(disconnection_mutex_);
    return disconnection_cv_.wait_for(lock, Config::SimpleBluez::disconnection_timeout, [this]() { return !is_connected(); });
}

std::shared_ptr<SimpleBluezLegacy::Characteristic> PeripheralLinuxLegacy::_get_characteristic(
    BluetoothUUID const& service_uuid, BluetoothUUID const& characteristic_uuid) {
    try {
        return device_->get_characteristic(service_uuid, characteristic_uuid);
    } catch (SimpleBluezLegacy::Exception::ServiceNotFoundException& e) {
        throw Exception::ServiceNotFound(service_uuid);
    } catch (SimpleBluezLegacy::Exception::CharacteristicNotFoundException& e) {
        throw Exception::CharacteristicNotFound(characteristic_uuid);
    }
}

std::shared_ptr<SimpleBluezLegacy::Descriptor> PeripheralLinuxLegacy::_get_descriptor(BluetoothUUID const& service_uuid,
                                                                          BluetoothUUID const& characteristic_uuid,
                                                                          BluetoothUUID const& descriptor_uuid) {
    try {
        return device_->get_characteristic(service_uuid, characteristic_uuid)->get_descriptor(descriptor_uuid);
    } catch (SimpleBluezLegacy::Exception::DescriptorNotFoundException& e) {
        throw Exception::DescriptorNotFound(descriptor_uuid);
    }
}
