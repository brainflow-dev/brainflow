#pragma once

#include <memory>

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include <BluezDevice.h>

#include <condition_variable>

namespace SimpleBLE {

class PeripheralBase {
  public:
    PeripheralBase(std::shared_ptr<BluezDevice> device);
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
    std::weak_ptr<BluezDevice> device_;

    std::string identifier_;
    BluetoothAddress address_;

    std::condition_variable connection_cv_;
    std::mutex connection_mutex_;

    std::condition_variable disconnection_cv_;
    std::mutex disconnection_mutex_;

    std::function<void()> callback_on_connected_;
    std::function<void()> callback_on_disconnected_;

    bool _attempt_connect();
    bool _attempt_disconnect();

    std::shared_ptr<BluezDevice> _get_device();

    std::shared_ptr<BluezGattCharacteristic> _get_characteristic(BluetoothUUID service_uuid,
                                                                 BluetoothUUID characteristic_uuid);
};

}  // namespace SimpleBLE
