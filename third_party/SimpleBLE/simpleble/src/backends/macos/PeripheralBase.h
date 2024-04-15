#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Service.h>
#include <simpleble/Types.h>

#include "AdapterBaseTypes.h"

#include <kvn_safe_callback.hpp>

#include <memory>

namespace SimpleBLE {

class PeripheralBase {
  public:
    PeripheralBase(void* opaque_peripheral, void* opaque_adapter, advertising_data_t advertising_data);
    ~PeripheralBase();

    void* underlying() const;

    std::string identifier();
    BluetoothAddress address();
    BluetoothAddressType address_type();
    int16_t tx_power();
    int16_t rssi();
    uint16_t mtu();

    void connect();
    void disconnect();
    bool is_connected();
    bool is_connectable();
    bool is_paired();
    void unpair();

    std::vector<Service> services();
    std::vector<Service> advertised_services();
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

    void delegate_did_connect();
    void delegate_did_fail_to_connect(void* opaque_error);
    void delegate_did_disconnect(void* opaque_error);

    void update_advertising_data(advertising_data_t advertising_data);

  protected:
    /**
     * Holds a pointer to the Objective-C representation of this object.
     */
    void* opaque_internal_;

    int16_t rssi_;
    int16_t tx_power_;

    bool is_connectable_;

    bool manual_disconnect_triggered_;

    std::map<uint16_t, ByteArray> manufacturer_data_;
    std::map<BluetoothUUID, ByteArray> service_data_;

    kvn::safe_callback<void()> callback_on_connected_;
    kvn::safe_callback<void()> callback_on_disconnected_;
};

}  // namespace SimpleBLE
