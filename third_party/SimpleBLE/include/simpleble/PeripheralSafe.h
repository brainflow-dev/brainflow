#pragma once

#include <optional>

#include <simpleble/Peripheral.h>

namespace SimpleBLE {

namespace Safe {

class Peripheral : public SimpleBLE::Peripheral {
  public:
    Peripheral(SimpleBLE::Peripheral& peripheral);
    virtual ~Peripheral() = default;

    std::optional<std::string> identifier() noexcept;
    std::optional<BluetoothAddress> address() noexcept;
    std::optional<int16_t> rssi() noexcept;

    bool connect() noexcept;
    bool disconnect() noexcept;
    std::optional<bool> is_connected() noexcept;
    std::optional<bool> is_connectable() noexcept;
    std::optional<bool> is_paired() noexcept;
    bool unpair() noexcept;

    std::optional<std::vector<BluetoothService>> services() noexcept;
    std::optional<std::map<uint16_t, ByteArray>> manufacturer_data() noexcept;

    std::optional<ByteArray> read(BluetoothUUID const& service, BluetoothUUID const& characteristic) noexcept;
    bool write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) noexcept;
    bool write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) noexcept;
    bool notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                std::function<void(ByteArray payload)> callback) noexcept;
    bool indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                  std::function<void(ByteArray payload)> callback) noexcept;
    bool unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) noexcept;

    bool set_callback_on_connected(std::function<void()> on_connected) noexcept;
    bool set_callback_on_disconnected(std::function<void()> on_disconnected) noexcept;
};

}  // namespace Safe

}  // namespace SimpleBLE
