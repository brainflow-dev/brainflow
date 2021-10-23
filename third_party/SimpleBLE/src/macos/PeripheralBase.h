#pragma once

#include <memory>

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>
#include "AdapterBaseTypes.h"

namespace SimpleBLE {

class PeripheralBase {
  public:
    PeripheralBase(void* opaque_peripheral, void* opaque_adapter, advertising_data_t advertising_data);
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

    void delegate_did_connect();
    void delegate_did_disconnect();

  protected:
    /**
     * Holds a pointer to the Objective-C representation of this object.
     */
    void* opaque_internal_;

    bool is_connectable_;

    bool manual_disconnect_triggered_;

    std::map<uint16_t, SimpleBLE::ByteArray> manufacturer_data_;

    std::function<void()> callback_on_connected_;
    std::function<void()> callback_on_disconnected_;
};

}  // namespace SimpleBLE
