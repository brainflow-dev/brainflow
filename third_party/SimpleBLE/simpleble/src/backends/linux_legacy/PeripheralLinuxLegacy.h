#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Service.h>
#include <simpleble/Types.h>

#include <simplebluezlegacy/Adapter.h>
#include <simplebluezlegacy/Characteristic.h>
#include <simplebluezlegacy/Device.h>

#include "../common/PeripheralBase.h"

#include <kvn_safe_callback.hpp>

#include <atomic>
#include <condition_variable>
#include <memory>

namespace SimpleBLE {

class PeripheralLinuxLegacy : public SimpleBLE::PeripheralBase {
  public:
    PeripheralLinuxLegacy(std::shared_ptr<SimpleBluezLegacy::Device> device, std::shared_ptr<SimpleBluezLegacy::Adapter> adapter);
    virtual ~PeripheralLinuxLegacy();

    virtual void* underlying() const override;

    virtual std::string identifier() override;
    virtual BluetoothAddress address() override;
    virtual BluetoothAddressType address_type() override;
    virtual int16_t rssi() override;

    virtual int16_t tx_power() override;
    virtual uint16_t mtu() override;

    virtual void connect() override;
    virtual void disconnect() override;
    virtual bool is_connected() override;
    virtual bool is_connectable() override;
    virtual bool is_paired() override;
    virtual void unpair() override;

    virtual std::vector<std::shared_ptr<ServiceBase>> available_services() override;
    virtual std::vector<std::shared_ptr<ServiceBase>> advertised_services() override;

    virtual std::map<uint16_t, ByteArray> manufacturer_data() override;

    // clang-format off
    virtual ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic) override;
    virtual void write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) override;
    virtual void write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) override;
    virtual void notify(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) override;
    virtual void indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic, std::function<void(ByteArray payload)> callback) override;
    virtual void unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) override;

    virtual ByteArray read(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor) override;
    virtual void write(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor, ByteArray const& data) override;
    // clang-format on

    virtual void set_callback_on_connected(std::function<void()> on_connected) override;
    virtual void set_callback_on_disconnected(std::function<void()> on_disconnected) override;

  private:
    std::atomic_bool battery_emulation_required_{false};

    std::shared_ptr<SimpleBluezLegacy::Adapter> adapter_;
    std::shared_ptr<SimpleBluezLegacy::Device> device_;

    std::condition_variable connection_cv_;
    std::mutex connection_mutex_;

    std::condition_variable disconnection_cv_;
    std::mutex disconnection_mutex_;

    kvn::safe_callback<void()> callback_on_connected_;
    kvn::safe_callback<void()> callback_on_disconnected_;

    bool _attempt_connect();
    bool _attempt_disconnect();
    void _cleanup_characteristics() noexcept;

    std::shared_ptr<SimpleBluezLegacy::Characteristic> _get_characteristic(BluetoothUUID const& service_uuid,
                                                                     BluetoothUUID const& characteristic_uuid);

    std::shared_ptr<SimpleBluezLegacy::Descriptor> _get_descriptor(BluetoothUUID const& service_uuid,
                                                             BluetoothUUID const& characteristic_uuid,
                                                             BluetoothUUID const& descriptor_uuid);
};

}  // namespace SimpleBLE
