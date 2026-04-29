#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Service.h>
#include <simpleble/Types.h>
#include "PeripheralBase.h"

#include <TaskRunner.hpp>
#include <kvn_safe_callback.hpp>
#include <kvn_safe_map.hpp>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

#include "AdapterBaseTypes.h"
#include "protocol/d2h.pb.h"
#include "serial/Protocol.h"

namespace SimpleBLE {

class PeripheralDongl : public PeripheralBase {
  public:
    PeripheralDongl(std::shared_ptr<Dongl::Serial::Protocol> serial_protocol, advertising_data_t advertising_data);
    virtual ~PeripheralDongl();

    void* underlying() const override;

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

    // Internal methods not exposed to the user.
    // TODO: Make these private and the adapter a friend.
    uint16_t conn_handle() const;
    void update_advertising_data(advertising_data_t advertising_data);
    void notify_connected(uint16_t conn_handle);
    void notify_disconnected();
    void notify_service_discovered(simpleble_ServiceDiscoveredEvt const& service_discovered_evt);
    void notify_characteristic_discovered(simpleble_CharacteristicDiscoveredEvt const& characteristic_discovered_evt);
    void notify_descriptor_discovered(simpleble_DescriptorDiscoveredEvt const& descriptor_discovered_evt);
    void notify_attribute_discovery_complete();
    void notify_value_changed(simpleble_ValueChangedEvt const& value_changed_evt);

    const uint16_t BLE_CONN_HANDLE_INVALID = 0xFFFF;
    const uint16_t BLE_CONN_HANDLE_PENDING = 0xFFFE;

  private:
    struct DescriptorDefinition {
        BluetoothUUID uuid;
        uint16_t handle;
    };

    struct CharacteristicDefinition {
        BluetoothUUID uuid;
        uint16_t handle_decl;
        uint16_t handle_value;
        uint16_t handle_cccd = 0;
        bool can_read;
        bool can_write_request;
        bool can_write_command;
        bool can_notify;
        bool can_indicate;

        std::vector<DescriptorDefinition> descriptors;
    };

    struct ServiceDefinition {
        BluetoothUUID uuid;
        uint16_t start_handle;
        uint16_t end_handle;
        std::vector<CharacteristicDefinition> characteristics;
    };

    bool _attempt_connect();
    BluetoothUUID _uuid_from_uuid16(uint16_t uuid16);
    BluetoothUUID _uuid_from_uuid32(uint32_t uuid32);
    BluetoothUUID _uuid_from_uuid128(const uint8_t uuid[16]);
    BluetoothUUID _uuid_from_proto(simpleble_UUID const& uuid);

    ServiceDefinition& _find_service_from_handle(uint16_t handle);
    CharacteristicDefinition& _find_characteristic_from_handle(uint16_t handle);
    CharacteristicDefinition& _find_characteristic_from_uuid(BluetoothUUID const& service,
                                                             BluetoothUUID const& characteristic);

    uint16_t _conn_handle = BLE_CONN_HANDLE_INVALID;
    std::string _identifier;
    BluetoothAddress _address;
    BluetoothAddressType _address_type;
    int16_t _rssi;
    int16_t _tx_power;
    bool _connectable;
    std::map<uint16_t, ByteArray> _manufacturer_data;
    std::map<BluetoothUUID, ByteArray> _service_data;

    std::vector<ServiceDefinition> _services;

    std::shared_ptr<Dongl::Serial::Protocol> _serial_protocol;

    std::condition_variable connection_cv_;
    std::mutex connection_mutex_;
    std::condition_variable disconnection_cv_;
    std::mutex disconnection_mutex_;
    std::condition_variable attributes_discovered_cv_;
    std::mutex attributes_discovered_mutex_;

    kvn::safe_callback<void()> _callback_on_connected;
    kvn::safe_callback<void()> _callback_on_disconnected;
    kvn::safe_map<uint16_t, std::function<void(ByteArray payload)>> _callbacks_on_value_changed;
};

}  // namespace SimpleBLE
