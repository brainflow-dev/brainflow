// This weird pragma is required for the compiler to properly include the necessary namespaces.
#pragma comment(lib, "windowsapp")

#include "PeripheralBase.h"
#include "CommonUtils.h"
#include "Utils.h"

#include <simpleble/Exceptions.h>

#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/base.h"

#include <iostream>

using namespace SimpleBLE;
using namespace std::chrono_literals;

PeripheralBase::PeripheralBase(advertising_data_t advertising_data) {
    identifier_ = advertising_data.identifier;
    address_ = advertising_data.mac_address;
    rssi_ = advertising_data.rssi;
    manufacturer_data_ = advertising_data.manufacturer_data;
    connectable_ = advertising_data.connectable;
    device_ = async_get(
        BluetoothLEDevice::FromBluetoothAddressAsync(_str_to_mac_address(advertising_data.mac_address)));
}

PeripheralBase::~PeripheralBase() {
    if (connection_status_changed_token_) {
        device_.ConnectionStatusChanged(connection_status_changed_token_);
    }
}

void* PeripheralBase::underlying() const { return reinterpret_cast<void*>(const_cast<BluetoothLEDevice*>(&device_)); }

std::string PeripheralBase::identifier() { return identifier_; }

BluetoothAddress PeripheralBase::address() { return address_; }

int16_t PeripheralBase::rssi() { return rssi_; }

void PeripheralBase::update_advertising_data(advertising_data_t advertising_data) {
    rssi_ = advertising_data.rssi;
    manufacturer_data_ = advertising_data.manufacturer_data;
}

void PeripheralBase::connect() {
    // Attempt to connect to the device.
    for (size_t i = 0; i < 3; i++) {
        if (_attempt_connect()) {
            break;
        }
    }

    if (is_connected()) {
        connection_status_changed_token_ = device_.ConnectionStatusChanged(
            [this](const BluetoothLEDevice device, const auto args) {
                if (device.ConnectionStatus() == BluetoothConnectionStatus::Disconnected) {
                    this->disconnection_cv_.notify_all();

                    SAFE_CALLBACK_CALL(this->callback_on_disconnected_);
                }
            });

        SAFE_CALLBACK_CALL(this->callback_on_connected_);
    }
}

void PeripheralBase::disconnect() {
    characteristics_map_.clear();
    if (device_ != nullptr) {
        device_.Close();
    }

    std::unique_lock<std::mutex> lock(disconnection_mutex_);
    if (disconnection_cv_.wait_for(lock, 5s, [=] { return !this->is_connected(); })) {
        // Disconnection successful
    } else {
        // Failed to disconnect
        // TODO: Raise an error
    }
}

bool PeripheralBase::is_connected() {
    return device_ != nullptr && device_.ConnectionStatus() == BluetoothConnectionStatus::Connected;
}

bool PeripheralBase::is_connectable() { return connectable_; }

bool PeripheralBase::is_paired() { throw Exception::OperationNotSupported(); }

void PeripheralBase::unpair() { throw Exception::OperationNotSupported(); }

std::vector<BluetoothService> PeripheralBase::services() {
    std::vector<BluetoothService> list_of_services;
    for (auto& service : characteristics_map_) {
        BluetoothService ble_service;
        ble_service.uuid = service.first;
        for (auto& characteristic : service.second) {
            ble_service.characteristics.push_back(characteristic.first);
        }
        list_of_services.push_back(ble_service);
    }
    return list_of_services;
}

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() { return manufacturer_data_; }

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic);

    // Validate that the operation can be performed.
    uint32_t gatt_characteristic_prop = (uint32_t)gatt_characteristic.CharacteristicProperties();
    if ((gatt_characteristic_prop & (uint32_t)GattCharacteristicProperties::Read) == 0) {
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    // Read the value.
    auto result = async_get(gatt_characteristic.ReadValueAsync(Devices::Bluetooth::BluetoothCacheMode::Uncached));
    if (result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
    return ibuffer_to_bytearray(result.Value());
}

void PeripheralBase::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                   ByteArray const& data) {
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic);

    // Validate that the operation can be performed.
    uint32_t gatt_characteristic_prop = (uint32_t)gatt_characteristic.CharacteristicProperties();
    if ((gatt_characteristic_prop & (uint32_t)GattCharacteristicProperties::Write) == 0) {
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    // Convert the request data to a buffer.
    winrt::Windows::Storage::Streams::IBuffer buffer = bytearray_to_ibuffer(data);

    // Write the value.
    auto result = async_get(gatt_characteristic.WriteValueAsync(buffer, GattWriteOption::WriteWithResponse));
    if (result != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
}

void PeripheralBase::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                   ByteArray const& data) {
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic);

    // Validate that the operation can be performed.
    uint32_t gatt_characteristic_prop = (uint32_t)gatt_characteristic.CharacteristicProperties();
    if ((gatt_characteristic_prop & (uint32_t)GattCharacteristicProperties::WriteWithoutResponse) == 0) {
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    // Convert the request data to a buffer.
    winrt::Windows::Storage::Streams::IBuffer buffer = bytearray_to_ibuffer(data);

    // Write the value.
    auto result = async_get(gatt_characteristic.WriteValueAsync(buffer, GattWriteOption::WriteWithoutResponse));
    if (result != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
}

void PeripheralBase::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                            std::function<void(ByteArray payload)> callback) {
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic);

    // Validate that the operation can be performed.
    uint32_t gatt_characteristic_prop = (uint32_t)gatt_characteristic.CharacteristicProperties();
    if ((gatt_characteristic_prop & (uint32_t)GattCharacteristicProperties::Notify) == 0) {
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    // Register the callback.
    gatt_characteristic.ValueChanged([=](const GattCharacteristic& sender, const GattValueChangedEventArgs& args) {
        // Convert the payload to a ByteArray.
        ByteArray payload = ibuffer_to_bytearray(args.CharacteristicValue());
        callback(payload);
    });

    // Start the notification.
    auto result = async_get(gatt_characteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(
        GattClientCharacteristicConfigurationDescriptorValue::Notify));

    if (result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
}

void PeripheralBase::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                              std::function<void(ByteArray payload)> callback) {
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic);

    // Validate that the operation can be performed.
    uint32_t gatt_characteristic_prop = (uint32_t)gatt_characteristic.CharacteristicProperties();
    if ((gatt_characteristic_prop & (uint32_t)GattCharacteristicProperties::Indicate) == 0) {
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    // Register the callback.
    gatt_characteristic.ValueChanged([=](const GattCharacteristic& sender, const GattValueChangedEventArgs& args) {
        // Convert the payload to a ByteArray.
        ByteArray payload = ibuffer_to_bytearray(args.CharacteristicValue());
        callback(payload);
    });

    // Start the indication.
    auto result = async_get(gatt_characteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(
        GattClientCharacteristicConfigurationDescriptorValue::Indicate));

    if (result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
}

void PeripheralBase::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic);

    // Start the indication.
    auto result = async_get(gatt_characteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(
        GattClientCharacteristicConfigurationDescriptorValue::None));

    if (result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
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

// Private methods

bool PeripheralBase::_attempt_connect() {
    characteristics_map_.clear();

    // We need to cache all services and characteristics in the class, else
    // the underlying objects will be garbage collected.
    auto services_result = async_get(device_.GetGattServicesAsync(BluetoothCacheMode::Uncached));

    // If reading services fails, raise an error.
    if (services_result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        return false;
    }

    auto gatt_services = services_result.Services();
    for (GattDeviceService&& service : gatt_services) {
        // For each service...
        std::string service_uuid = guid_to_uuid(service.Uuid());
        auto characteristics_result = async_get(service.GetCharacteristicsAsync(BluetoothCacheMode::Uncached));

        // If reading characteristics fails, raise an error.
        if (characteristics_result.Status() != GattCommunicationStatus::Success) {
            return false;
        }

        auto gatt_characteristics = characteristics_result.Characteristics();
        for (GattCharacteristic&& characteristic : gatt_characteristics) {
            // For each characteristic...
            // Store the underlying object pointer in a map.
            std::string characteristic_uuid = guid_to_uuid(characteristic.Uuid());
            characteristics_map_[service_uuid].emplace(characteristic_uuid, characteristic);
        }
    }
    return true;
}

GattCharacteristic PeripheralBase::_fetch_characteristic(const BluetoothUUID& service_uuid,
                                                         const BluetoothUUID& characteristic_uuid) {
    if (characteristics_map_.count(service_uuid) == 0) {
        throw SimpleBLE::Exception::ServiceNotFound(service_uuid);
    }

    if (characteristics_map_[service_uuid].count(characteristic_uuid) == 0) {
        throw SimpleBLE::Exception::CharacteristicNotFound(characteristic_uuid);
    }

    return characteristics_map_[service_uuid].at(characteristic_uuid);
}
