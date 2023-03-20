// This weird pragma is required for the compiler to properly include the necessary namespaces.
#pragma comment(lib, "windowsapp")

#include "PeripheralBase.h"
#include "CommonUtils.h"
#include "Utils.h"

#include "CharacteristicBuilder.h"
#include "DescriptorBuilder.h"
#include "ServiceBuilder.h"

#include <simpleble/Exceptions.h>

#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Storage.Streams.h"
#include "winrt/base.h"

#include <iostream>

using namespace SimpleBLE;
using namespace std::chrono_literals;

PeripheralBase::PeripheralBase(advertising_data_t advertising_data) {
    address_type_ = advertising_data.address_type;
    identifier_ = advertising_data.identifier;
    address_ = advertising_data.mac_address;
    rssi_ = advertising_data.rssi;
    tx_power_ = advertising_data.tx_power;
    manufacturer_data_ = advertising_data.manufacturer_data;
    service_data_ = advertising_data.service_data;
    connectable_ = advertising_data.connectable;
}

PeripheralBase::~PeripheralBase() {
    if (connection_status_changed_token_ && device_ != nullptr) {
        device_.ConnectionStatusChanged(connection_status_changed_token_);
    }
}

void* PeripheralBase::underlying() const { return reinterpret_cast<void*>(const_cast<BluetoothLEDevice*>(&device_)); }

SimpleBLE::BluetoothAddressType PeripheralBase::address_type() { return address_type_; }

std::string PeripheralBase::identifier() { return identifier_; }

BluetoothAddress PeripheralBase::address() { return address_; }

int16_t PeripheralBase::rssi() { return rssi_; }

int16_t PeripheralBase::tx_power() { return tx_power_; }

uint16_t PeripheralBase::mtu() {
    if (!is_connected()) return 0;

    // The value provided by the MaxPduSize includes an extra 3 bytes from the GATT header
    // which needs to be removed.
    return mtu_ - 3;
}

void PeripheralBase::update_advertising_data(advertising_data_t advertising_data) {
    if (advertising_data.identifier != "") {
        identifier_ = advertising_data.identifier;
    }
    rssi_ = advertising_data.rssi;
    tx_power_ = advertising_data.tx_power;
    address_type_ = advertising_data.address_type;
    manufacturer_data_ = advertising_data.manufacturer_data;

    advertising_data.service_data.merge(service_data_);
    service_data_ = advertising_data.service_data;
}

void PeripheralBase::connect() {
    device_ = async_get(BluetoothLEDevice::FromBluetoothAddressAsync(_str_to_mac_address(address_)));

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
    } else {
        throw SimpleBLE::Exception::OperationFailed("Failed to connect to device.");
    }
}

void PeripheralBase::disconnect() {
    gatt_map_.clear();
    if (device_ != nullptr) {
        device_.Close();

        std::unique_lock<std::mutex> lock(disconnection_mutex_);
        if (disconnection_cv_.wait_for(lock, 10s, [=] { return !this->is_connected(); })) {
            // Disconnection successful
        } else {
            SIMPLEBLE_LOG_ERROR("Disconnection failed");
            throw SimpleBLE::Exception::OperationFailed("Disconnection attempt was not acknowledged.");
        }

        device_ = nullptr;
    }
}

bool PeripheralBase::is_connected() {
    return device_ != nullptr && device_.ConnectionStatus() == BluetoothConnectionStatus::Connected;
}

bool PeripheralBase::is_connectable() { return connectable_; }

bool PeripheralBase::is_paired() { throw Exception::OperationNotSupported(); }

void PeripheralBase::unpair() { throw Exception::OperationNotSupported(); }

std::vector<Service> PeripheralBase::services() {
    std::vector<Service> service_list;
    for (auto& [service_uuid, service] : gatt_map_) {
        // Build the list of characteristics for the service.
        std::vector<Characteristic> characteristic_list;
        for (auto& [characteristic_uuid, characteristic] : service.characteristics) {
            // Build the list of descriptors for the characteristic.
            std::vector<Descriptor> descriptor_list;
            for (auto& [descriptor_uuid, descriptor] : characteristic.descriptors) {
                descriptor_list.push_back(DescriptorBuilder(descriptor_uuid));
            }

            uint32_t properties = (uint32_t)characteristic.obj.CharacteristicProperties();
            bool can_read = (properties & (uint32_t)GattCharacteristicProperties::Read) != 0;
            bool can_write_request = (properties & (uint32_t)GattCharacteristicProperties::Write) != 0;
            bool can_write_command = (properties & (uint32_t)GattCharacteristicProperties::WriteWithoutResponse) != 0;
            bool can_notify = (properties & (uint32_t)GattCharacteristicProperties::Notify) != 0;
            bool can_indicate = (properties & (uint32_t)GattCharacteristicProperties::Indicate) != 0;

            characteristic_list.push_back(CharacteristicBuilder(characteristic_uuid, descriptor_list, can_read,
                                                                can_write_request, can_write_command, can_notify,
                                                                can_indicate));
        }
        service_list.push_back(ServiceBuilder(service_uuid, characteristic_list));
    }

    return service_list;
}

std::vector<Service> PeripheralBase::advertised_services() {
    std::vector<Service> service_list;
    for (auto& [service_uuid, data] : service_data_) {
        service_list.push_back(ServiceBuilder(service_uuid, data));
    }

    return service_list;
}

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() { return manufacturer_data_; }

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic).obj;

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
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic).obj;

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
    GattCharacteristic gatt_characteristic = _fetch_characteristic(service, characteristic).obj;

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
    _subscribe(service, characteristic, std::move(callback), GattCharacteristicProperties::Notify,
               GattClientCharacteristicConfigurationDescriptorValue::Notify);
}

void PeripheralBase::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                              std::function<void(ByteArray payload)> callback) {
    _subscribe(service, characteristic, std::move(callback), GattCharacteristicProperties::Indicate,
               GattClientCharacteristicConfigurationDescriptorValue::Indicate);
}

void PeripheralBase::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    gatt_characteristic_t& gatt_characteristic_holder = _fetch_characteristic(service, characteristic);
    GattCharacteristic gatt_characteristic = gatt_characteristic_holder.obj;

    if (gatt_characteristic_holder.value_changed_token) {
        // Unregister the callback.
        gatt_characteristic.ValueChanged(gatt_characteristic_holder.value_changed_token);
        gatt_characteristic_holder.value_changed_token = {0};
    }

    // Start the indication.
    auto result = async_get(gatt_characteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(
        GattClientCharacteristicConfigurationDescriptorValue::None));

    if (result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
}

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                               BluetoothUUID const& descriptor) {
    GattDescriptor gatt_descriptor = _fetch_descriptor(service, characteristic, descriptor);

    // Read the value.
    auto result = async_get(gatt_descriptor.ReadValueAsync(Devices::Bluetooth::BluetoothCacheMode::Uncached));
    if (result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
    return ibuffer_to_bytearray(result.Value());
}

void PeripheralBase::write(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                           BluetoothUUID const& descriptor, ByteArray const& data) {
    GattDescriptor gatt_descriptor = _fetch_descriptor(service, characteristic, descriptor);

    // Convert the request data to a buffer.
    winrt::Windows::Storage::Streams::IBuffer buffer = bytearray_to_ibuffer(data);

    // Write the value.
    auto result = async_get(gatt_descriptor.WriteValueWithResultAsync(buffer));
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

void PeripheralBase::_subscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                                std::function<void(ByteArray payload)> callback, GattCharacteristicProperties property,
                                GattClientCharacteristicConfigurationDescriptorValue descriptor_value) {
    gatt_characteristic_t& gatt_characteristic_holder = _fetch_characteristic(service, characteristic);
    GattCharacteristic gatt_characteristic = gatt_characteristic_holder.obj;

    // Validate that the operation can be performed.
    uint32_t gatt_characteristic_prop = (uint32_t)gatt_characteristic.CharacteristicProperties();
    if ((gatt_characteristic_prop & (uint32_t)property) == 0) {
        throw SimpleBLE::Exception::OperationNotSupported();
    }

    // If a notification for the given characteristic is already in progress, swap the callbacks.
    if (gatt_characteristic_holder.value_changed_token) {
        SIMPLEBLE_LOG_WARN("A notification for the given characteristic is already in progress. Swapping callbacks.");
        // Unregister the callback.
        gatt_characteristic.ValueChanged(gatt_characteristic_holder.value_changed_token);
        gatt_characteristic_holder.value_changed_token = {0};
    }

    gatt_characteristic_holder.value_changed_callback = [=](const GattCharacteristic& sender,
                                                            const GattValueChangedEventArgs& args) {
        // Convert the payload to a ByteArray.
        ByteArray payload = ibuffer_to_bytearray(args.CharacteristicValue());
        callback(payload);
    };

    // Register the callback.
    gatt_characteristic_holder.value_changed_token = gatt_characteristic.ValueChanged(
        gatt_characteristic_holder.value_changed_callback);

    // Start the notification.
    auto result = async_get(
        gatt_characteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(descriptor_value));

    if (result.Status() != GenericAttributeProfile::GattCommunicationStatus::Success) {
        throw SimpleBLE::Exception::OperationFailed();
    }
}

bool PeripheralBase::_attempt_connect() {
    gatt_map_.clear();

    // We need to cache all services, characteristics and descriptors in the class, else
    // the underlying objects will be garbage collected.
    auto services_result = async_get(device_.GetGattServicesAsync(BluetoothCacheMode::Uncached));
    if (services_result.Status() != GattCommunicationStatus::Success) {
        return false;
    }

    auto gatt_services = services_result.Services();
    for (GattDeviceService&& service : gatt_services) {
        // For each service...
        gatt_service_t gatt_service;
        gatt_service.obj = service;

        // Save the MTU size
        mtu_ = service.Session().MaxPduSize();

        // Fetch the service UUID
        std::string service_uuid = guid_to_uuid(service.Uuid());

        // Fetch the service characteristics
        auto characteristics_result = async_get(service.GetCharacteristicsAsync(BluetoothCacheMode::Uncached));
        if (characteristics_result.Status() != GattCommunicationStatus::Success) {
            return false;
        }

        // Load the characteristics into the service
        auto gatt_characteristics = characteristics_result.Characteristics();
        for (GattCharacteristic&& characteristic : gatt_characteristics) {
            // For each characteristic...
            gatt_characteristic_t gatt_characteristic;
            gatt_characteristic.obj = characteristic;

            // Fetch the characteristic UUID
            std::string characteristic_uuid = guid_to_uuid(characteristic.Uuid());

            // Fetch the characteristic descriptors
            auto descriptors_result = async_get(characteristic.GetDescriptorsAsync(BluetoothCacheMode::Uncached));
            if (descriptors_result.Status() != GattCommunicationStatus::Success) {
                return false;
            }

            // Load the descriptors into the characteristic
            auto gatt_descriptors = descriptors_result.Descriptors();
            for (GattDescriptor&& descriptor : gatt_descriptors) {
                // For each descriptor...
                gatt_descriptor_t gatt_descriptor;
                gatt_descriptor.obj = descriptor;

                // Fetch the descriptor UUID.
                std::string descriptor_uuid = guid_to_uuid(descriptor.Uuid());

                // Append the descriptor to the characteristic.
                gatt_characteristic.descriptors.emplace(descriptor_uuid, std::move(gatt_descriptor));
            }

            // Append the characteristic to the service.
            gatt_service.characteristics.emplace(characteristic_uuid, std::move(gatt_characteristic));
        }

        // Append the service to the map.
        gatt_map_.emplace(service_uuid, std::move(gatt_service));
    }

    return true;
}

gatt_characteristic_t& PeripheralBase::_fetch_characteristic(const BluetoothUUID& service_uuid,
                                                             const BluetoothUUID& characteristic_uuid) {
    if (gatt_map_.count(service_uuid) == 0) {
        throw SimpleBLE::Exception::ServiceNotFound(service_uuid);
    }

    if (gatt_map_[service_uuid].characteristics.count(characteristic_uuid) == 0) {
        throw SimpleBLE::Exception::CharacteristicNotFound(characteristic_uuid);
    }

    return gatt_map_[service_uuid].characteristics.at(characteristic_uuid);
}

GattDescriptor PeripheralBase::_fetch_descriptor(const BluetoothUUID& service_uuid,
                                                 const BluetoothUUID& characteristic_uuid,
                                                 const BluetoothUUID& descriptor_uuid) {
    if (gatt_map_.count(service_uuid) == 0) {
        throw SimpleBLE::Exception::ServiceNotFound(service_uuid);
    }

    if (gatt_map_[service_uuid].characteristics.count(characteristic_uuid) == 0) {
        throw SimpleBLE::Exception::CharacteristicNotFound(characteristic_uuid);
    }

    if (gatt_map_[service_uuid].characteristics[characteristic_uuid].descriptors.count(descriptor_uuid) == 0) {
        throw SimpleBLE::Exception::DescriptorNotFound(descriptor_uuid);
    }

    return gatt_map_[service_uuid].characteristics[characteristic_uuid].descriptors.at(descriptor_uuid).obj;
}
