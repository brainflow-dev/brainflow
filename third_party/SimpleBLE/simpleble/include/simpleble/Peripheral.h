#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <simpleble/export.h>

#include <simpleble/Exceptions.h>
#include <simpleble/Service.h>
#include <simpleble/Types.h>

namespace SimpleBLE {

class PeripheralBase;

class SIMPLEBLE_EXPORT Peripheral {
  public:
    Peripheral() = default;
    virtual ~Peripheral() = default;

    bool initialized() const;
    void* underlying() const;

    std::string identifier();
    BluetoothAddress address();
    BluetoothAddressType address_type();
    int16_t rssi();

    /**
     * @brief Provides the advertised transmit power in dBm.
     *
     * @note If the field has not been advertised by the peripheral,
     *       the returned value will be -32768.
     */
    int16_t tx_power();
    uint16_t mtu();

    void connect();
    void disconnect();
    bool is_connected();
    bool is_connectable();
    bool is_paired();
    void unpair();

    /**
     * @brief Provides a list of all services that are available on the peripheral.
     *
     * @note If the peripheral is not connected, it will return a list of services
     *       that were advertised by the device.
     */
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

  protected:
    std::shared_ptr<PeripheralBase> internal_;
};

}  // namespace SimpleBLE
