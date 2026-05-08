#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include "AdapterBaseTypes.h"
#include "PeripheralBase.h"

#include <kvn_safe_callback.hpp>

#include <memory>

namespace SimpleBLE {

class PeripheralMac : public PeripheralBase {
  public:
    PeripheralMac(void* opaque_peripheral, void* opaque_adapter, advertising_data_t advertising_data);
    ~PeripheralMac();

    virtual void* underlying() const override;

    virtual std::string identifier() override;
    virtual BluetoothAddress address() override;
    virtual BluetoothAddressType address_type() override;
    virtual int16_t tx_power() override;
    virtual int16_t rssi() override;
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
