#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include <simplebluez/Adapter.h>
#include <simplebluez/Characteristic.h>
#include <simplebluez/Device.h>

#include <kvn_safe_callback.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>

namespace SimpleBLE {

class PeripheralBase {
  public:
    PeripheralBase(std::shared_ptr<SimpleBluez::Device> device, std::shared_ptr<SimpleBluez::Adapter> adapter);
    virtual ~PeripheralBase();

    std::string identifier();
    BluetoothAddress address();

    void connect();
    void disconnect();
    bool is_connected();
    bool is_connectable();
    void unpair();

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
    std::atomic_bool battery_emulation_required_{false};

    std::shared_ptr<SimpleBluez::Adapter> adapter_;
    std::shared_ptr<SimpleBluez::Device> device_;

    std::condition_variable connection_cv_;
    std::mutex connection_mutex_;

    std::condition_variable disconnection_cv_;
    std::mutex disconnection_mutex_;

    kvn::safe_callback<void()> callback_on_connected_;
    kvn::safe_callback<void()> callback_on_disconnected_;

    bool _attempt_connect();
    bool _attempt_disconnect();

    std::shared_ptr<SimpleBluez::Characteristic> _get_characteristic(BluetoothUUID service_uuid,
                                                                     BluetoothUUID characteristic_uuid);
};

}  // namespace SimpleBLE
