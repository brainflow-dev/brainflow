#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include "AdapterBaseTypes.h"

#include "winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h"
#include "winrt/Windows.Devices.Bluetooth.h"

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

namespace SimpleBLE {

class PeripheralBase {
  public:
    PeripheralBase(advertising_data_t advertising_data);
    ~PeripheralBase();

    std::string identifier();
    BluetoothAddress address();

    void connect();
    void disconnect();
    bool is_connected();
    bool is_connectable();

    std::vector<BluetoothService> services();
    std::map<uint16_t, ByteArray> manufacturer_data();

    ByteArray read(BluetoothUUID service, BluetoothUUID characteristic);
    void write_request(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data);
    void write_command(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data);
    void notify(BluetoothUUID service, BluetoothUUID characteristic, std::function<void(ByteArray payload)> callback);
    void indicate(BluetoothUUID service, BluetoothUUID characteristic, std::function<void(ByteArray payload)> callback);
    void unsubscribe(BluetoothUUID service, BluetoothUUID characteristic);

    void set_callback_on_connected(std::function<void()> on_connected);
    void set_callback_on_disconnected(std::function<void()> on_disconnected);

  private:
    BluetoothLEDevice device_ = nullptr;

    // NOTE: Calling device_.Name() or device_.BluetoothAddress() might
    // cause a crash on some devices.
    // This is because any operation on the object before it is connected will
    // initiate a connection, which can then cause further cascading failures.
    // See:
    // https://docs.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothledevice.frombluetoothaddressasync
    std::string identifier_;
    BluetoothAddress address_;
    bool connectable_;

    std::condition_variable disconnection_cv_;
    std::mutex disconnection_mutex_;
    std::map<BluetoothUUID, std::map<BluetoothUUID, GattCharacteristic>> characteristics_map_;

    std::function<void()> callback_on_connected_;
    std::function<void()> callback_on_disconnected_;

    std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data_;

    bool _attempt_connect();
    GattCharacteristic _fetch_characteristic(const BluetoothUUID& service_uuid,
                                             const BluetoothUUID& characteristic_uuid);
};

}  // namespace SimpleBLE
