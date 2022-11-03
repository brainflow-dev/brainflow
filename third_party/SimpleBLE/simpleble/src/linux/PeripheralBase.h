#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Service.h>
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

    std::vector<Service> services();
    std::map<uint16_t, ByteArray> manufacturer_data();

    // clang-format off
    ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic);
    void write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data);
    void write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data);
    void notify(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback);
    void indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback);
    void unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic);

    ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor);
    void write(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor, ByteArray const& data);
    // clang-format on

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
    void _cleanup_characteristics() noexcept;

    std::shared_ptr<SimpleBluez::Characteristic> _get_characteristic(BluetoothUUID const& service_uuid,
                                                                     BluetoothUUID const& characteristic_uuid);

    std::shared_ptr<SimpleBluez::Descriptor> _get_descriptor(BluetoothUUID const& service_uuid,
                                                             BluetoothUUID const& characteristic_uuid,
                                                             BluetoothUUID const& descriptor_uuid);
};

}  // namespace SimpleBLE
