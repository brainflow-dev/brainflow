#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Service.h>
#include <simpleble/Types.h>

#include "AdapterBaseTypes.h"
#include "PeripheralBase.h"

#include <kvn_safe_callback.hpp>

#include "winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h"
#include "winrt/Windows.Devices.Bluetooth.h"

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

namespace SimpleBLE {

struct gatt_descriptor_t {
    GattDescriptor obj = nullptr;
};

struct gatt_characteristic_t {
    GattCharacteristic obj = nullptr;
    winrt::event_token value_changed_token;
    std::function<void(const GattCharacteristic& sender, const GattValueChangedEventArgs& args)> value_changed_callback;
    std::map<BluetoothUUID, gatt_descriptor_t> descriptors;
};

struct gatt_service_t {
    GattDeviceService obj = nullptr;
    std::map<BluetoothUUID, gatt_characteristic_t> characteristics;
};

class PeripheralWindows : public PeripheralBase {
  public:
    PeripheralWindows(BluetoothLEDevice device);
    PeripheralWindows(advertising_data_t advertising_data);
    virtual ~PeripheralWindows();

    virtual void* underlying() const override;

    virtual std::string identifier() override;
    virtual BluetoothAddress address() override;
    virtual SimpleBLE::BluetoothAddressType address_type() override;
    virtual int16_t rssi() override;
    virtual int16_t tx_power() override;
    virtual uint16_t mtu() override;

    virtual void connect() override;
    virtual void disconnect() override;
    virtual bool is_connected() override;
    virtual bool is_connectable() override;
    virtual bool is_paired() override;
    virtual void unpair() override;

    virtual std::vector<std::shared_ptr<ServiceBase>> available_services() override;
    virtual std::vector<std::shared_ptr<ServiceBase>> advertised_services() override;
    virtual std::map<uint16_t, ByteArray> manufacturer_data() override;

    // clang-format off
    virtual ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic) override;
    virtual void write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) override;
    virtual void write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) override;
    virtual void notify(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) override;
    virtual void indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) override;
    virtual void unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) override;

    virtual ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor) override;
    virtual void write(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor, ByteArray const& data) override;
    // clang-format on

    virtual void set_callback_on_connected(std::function<void()> on_connected) override;
    virtual void set_callback_on_disconnected(std::function<void()> on_disconnected) override;

    // Internal methods not exposed to the user.

    void update_advertising_data(advertising_data_t advertising_data);

  private:
    BluetoothLEDevice device_{nullptr};

    // NOTE: Calling device_.Name() or device_.BluetoothAddress() might
    // cause a crash on some devices.
    // This is because any operation on the object before it is connected will
    // initiate a connection, which can then cause further cascading failures.
    // See:
    // https://docs.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothledevice.frombluetoothaddressasync

    std::string identifier_;
    BluetoothAddress address_;
    SimpleBLE::BluetoothAddressType address_type_;
    int16_t rssi_ = INT16_MIN;
    int16_t tx_power_ = INT16_MIN;
    uint16_t mtu_;
    bool connectable_;
    winrt::event_token connection_status_changed_token_;

    std::condition_variable disconnection_cv_;
    std::mutex disconnection_mutex_;

    std::map<BluetoothUUID, gatt_service_t> gatt_map_;

    kvn::safe_callback<void()> callback_on_connected_;
    kvn::safe_callback<void()> callback_on_disconnected_;

    std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data_;
    std::map<BluetoothUUID, SimpleBLE::ByteArray> service_data_;

    bool _attempt_connect();

    gatt_characteristic_t& _fetch_characteristic(const BluetoothUUID& service_uuid,
                                                 const BluetoothUUID& characteristic_uuid);

    GattDescriptor _fetch_descriptor(const BluetoothUUID& service_uuid, const BluetoothUUID& characteristic_uuid,
                                     const BluetoothUUID& descriptor_uuid);

    void _subscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                    std::function<void(ByteArray payload)> callback, GattCharacteristicProperties property,
                    GattClientCharacteristicConfigurationDescriptorValue descriptor_value);
};

}  // namespace SimpleBLE
