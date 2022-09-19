#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include "AdapterBaseTypes.h"

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

class PeripheralBase {
  public:
    PeripheralBase(advertising_data_t advertising_data);
    virtual ~PeripheralBase();

    void* underlying() const;

    std::string identifier();
    BluetoothAddress address();
    int16_t rssi();

    void connect();
    void disconnect();
    bool is_connected();
    bool is_connectable();
    bool is_paired();
    void unpair();

    std::vector<BluetoothService> services();
    std::map<uint16_t, ByteArray> manufacturer_data();

    ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic);
    void write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data);
    void write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data);
    void notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                std::function<void(ByteArray payload)> callback);
    void indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                  std::function<void(ByteArray payload)> callback);
    void unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic);

    void set_callback_on_connected(std::function<void()> on_connected);
    void set_callback_on_disconnected(std::function<void()> on_disconnected);

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
    int16_t rssi_;
    bool connectable_;
    winrt::event_token connection_status_changed_token_;

    std::condition_variable disconnection_cv_;
    std::mutex disconnection_mutex_;
    std::map<BluetoothUUID, std::map<BluetoothUUID, GattCharacteristic>> characteristics_map_;

    kvn::safe_callback<void()> callback_on_connected_;
    kvn::safe_callback<void()> callback_on_disconnected_;

    std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data_;

    bool _attempt_connect();
    GattCharacteristic _fetch_characteristic(const BluetoothUUID& service_uuid,
                                             const BluetoothUUID& characteristic_uuid);
};

}  // namespace SimpleBLE
