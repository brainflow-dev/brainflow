#pragma once

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include <simpleble/Types.h>

namespace SimpleBLE {

class ServiceBase;

/**
 * Abstract base class for Bluetooth adapter implementations.
 *
 * Each backend must implement this class.
 *
 * Notes for implementers: ...
 */
class PeripheralBase {
  public:
    virtual ~PeripheralBase() = default;

    virtual void* underlying() const = 0;

    virtual std::string identifier() = 0;
    virtual BluetoothAddress address() = 0;
    virtual BluetoothAddressType address_type() = 0;
    virtual int16_t rssi() = 0;

    /**
     * @brief Provides the advertised transmit power in dBm.
     *
     * @note If the field has not been advertised by the peripheral,
     *       the returned value will be -32768.
     */
    virtual int16_t tx_power() = 0;
    virtual uint16_t mtu() = 0;

    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() = 0;
    virtual bool is_connectable() = 0;
    virtual bool is_paired() = 0;
    virtual void unpair() = 0;

    /**
     * Available services (if the peripheral is connected).
     */
    virtual std::vector<std::shared_ptr<ServiceBase>> available_services() = 0;

    /**
     * Advertised services (if the peripheral is not connected).
     */
    virtual std::vector<std::shared_ptr<ServiceBase>> advertised_services() = 0;

    virtual std::map<uint16_t, ByteArray> manufacturer_data() = 0;

    // clang-format off
    /* These methods are called by the frontend ONLY when the device is connected.
    */
    // clang-format off
    virtual ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic) = 0;
    virtual void write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) = 0;
    virtual void write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) = 0;
    virtual void notify(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) = 0;
    virtual void indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) = 0;
    virtual void unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) = 0;

    virtual ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor) = 0;
    virtual void write(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor, ByteArray const& data) = 0;
    // clang-format on

    virtual void set_callback_on_connected(std::function<void()> on_connected) = 0;
    virtual void set_callback_on_disconnected(std::function<void()> on_disconnected) = 0;

  protected:
    PeripheralBase() = default;
};

}  // namespace SimpleBLE
