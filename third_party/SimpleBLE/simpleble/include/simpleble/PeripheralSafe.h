#pragma once

#include <memory>
#include <optional>

#include <simpleble/export.h>

#include <simpleble/Peripheral.h>
#include <simpleble/Service.h>

namespace SimpleBLE {

namespace Safe {

class SIMPLEBLE_EXPORT Peripheral {
  public:
    Peripheral(SimpleBLE::Peripheral& peripheral);
    Peripheral(SimpleBLE::Peripheral&& peripheral);
    virtual ~Peripheral() = default;

    std::optional<std::string> identifier() noexcept;
    std::optional<BluetoothAddress> address() noexcept;
    std::optional<BluetoothAddressType> address_type() noexcept;
    std::optional<int16_t> rssi() noexcept;
    std::optional<int16_t> tx_power() noexcept;
    std::optional<uint16_t> mtu() noexcept;

    bool connect() noexcept;
    bool disconnect() noexcept;
    std::optional<bool> is_connected() noexcept;
    std::optional<bool> is_connectable() noexcept;
    std::optional<bool> is_paired() noexcept;
    bool unpair() noexcept;

    std::optional<std::vector<Service>> services() noexcept;
    std::optional<std::map<uint16_t, ByteArray>> manufacturer_data() noexcept;

    // clang-format off
    std::optional<ByteArray> read(BluetoothUUID const& service, BluetoothUUID const& characteristic) noexcept;
    bool write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) noexcept;
    bool write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) noexcept;
    bool notify(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) noexcept;
    bool indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) noexcept;
    bool unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) noexcept;

    std::optional<ByteArray> read(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor) noexcept;
    bool write(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor, ByteArray const& data) noexcept;
    // clang-format on

    bool set_callback_on_connected(std::function<void()> on_connected) noexcept;
    bool set_callback_on_disconnected(std::function<void()> on_disconnected) noexcept;

    /**
     * Get the underlying peripheral object.
     */
    operator SimpleBLE::Peripheral() const noexcept;

  protected:
    SimpleBLE::Peripheral internal_;
};

}  // namespace Safe

}  // namespace SimpleBLE
