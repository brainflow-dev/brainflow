#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class PeripheralBase;

class Peripheral {
  public:
    Peripheral();
    virtual ~Peripheral();

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

  protected:
    std::shared_ptr<PeripheralBase> internal_;
};

}  // namespace SimpleBLE
