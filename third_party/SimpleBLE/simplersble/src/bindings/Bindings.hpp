#pragma once

#include "rust/cxx.h"
#include "simpleble/Adapter.h"
#include "simpleble/Peripheral.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

namespace SimpleRsBLE {

struct InnerAdapter;
struct InnerPeripheral;

};  // namespace SimpleRsBLE

namespace Bindings {

struct RustyAdapterWrapper;
struct RustyPeripheralWrapper;
struct RustyServiceWrapper;
struct RustyCharacteristicWrapper;
struct RustyDescriptorWrapper;
struct RustyManufacturerDataWrapper;

rust::Vec<Bindings::RustyAdapterWrapper> RustyAdapter_get_adapters();
bool RustyAdapter_bluetooth_enabled();

class RustyAdapter {
  public:
    RustyAdapter() = default;
    virtual ~RustyAdapter() { _internal.reset(); }

    RustyAdapter(SimpleBLE::Adapter adapter) : _internal(new SimpleBLE::Adapter(std::move(adapter))) {};

    void link(SimpleRsBLE::InnerAdapter& target) const;
    void unlink() const;

    bool initialized() const;
    rust::String identifier() const;
    rust::String address() const;

    void power_on() const;
    void power_off() const;
    bool is_powered() const;

    void scan_start() const;
    void scan_stop() const;
    void scan_for(int32_t timeout_ms) const;
    bool scan_is_active() const;
    rust::Vec<Bindings::RustyPeripheralWrapper> scan_get_results() const;

    rust::Vec<Bindings::RustyPeripheralWrapper> get_paired_peripherals() const;
    rust::Vec<Bindings::RustyPeripheralWrapper> get_connected_peripherals() const;

  private:
    // NOTE: All internal properties need to be handled as pointers,
    // allowing the calls to RustyAdapter to always be const.
    // This might require us to store pointers to pointers, so it's
    // important to be careful when handling these.
    std::unique_ptr<SimpleBLE::Adapter> _internal;
    mutable SimpleRsBLE::InnerAdapter* _adapter = nullptr;
    mutable std::mutex _adapter_mutex;
};

class RustyPeripheral {
  public:
    RustyPeripheral() = default;
    virtual ~RustyPeripheral() { _internal.reset(); }

    RustyPeripheral(SimpleBLE::Peripheral peripheral) : _internal(new SimpleBLE::Peripheral(peripheral)) {}

    void link(SimpleRsBLE::InnerPeripheral& target) const;
    void unlink() const;

    bool initialized() const;
    rust::String identifier() const;
    rust::String address() const;
    SimpleBLE::BluetoothAddressType address_type() const;
    int16_t rssi() const;

    int16_t tx_power() const;
    uint16_t mtu() const;

    void connect() const;
    void disconnect() const;
    bool is_connected() const;
    bool is_connectable() const;
    bool is_paired() const;
    void unpair() const;

    rust::Vec<Bindings::RustyServiceWrapper> services() const;
    rust::Vec<Bindings::RustyManufacturerDataWrapper> manufacturer_data() const;

    rust::Vec<uint8_t> read(rust::String const& service, rust::String const& characteristic) const;
    void write_request(rust::String const& service, rust::String const& characteristic,
                       rust::Vec<uint8_t> const& data) const;
    void write_command(rust::String const& service, rust::String const& characteristic,
                       rust::Vec<uint8_t> const& data) const;
    void notify(rust::String const& service, rust::String const& characteristic) const;
    void indicate(rust::String const& service, rust::String const& characteristic) const;
    void unsubscribe(rust::String const& service, rust::String const& characteristic) const;

    rust::Vec<uint8_t> read_descriptor(rust::String const& service, rust::String const& characteristic,
                                       rust::String const& descriptor) const;
    void write_descriptor(rust::String const& service, rust::String const& characteristic,
                          rust::String const& descriptor, rust::Vec<uint8_t> const& data) const;

  private:
    // NOTE: All internal properties need to be handled as pointers,
    // allowing the calls to RustyPeripheral to always be const.
    // This might require us to store pointers to pointers, so it's
    // important to be careful when handling these.
    std::unique_ptr<SimpleBLE::Peripheral> _internal;
    mutable SimpleRsBLE::InnerPeripheral* _peripheral = nullptr;
    mutable std::mutex _peripheral_mutex;
};

class RustyService {
  public:
    RustyService() = default;
    virtual ~RustyService() = default;

    RustyService(SimpleBLE::Service service) : _internal(new SimpleBLE::Service(std::move(service))) {}

    bool initialized() const { return _internal->initialized(); }
    rust::String uuid() const { return rust::String(_internal->uuid()); }

    rust::Vec<uint8_t> data() const;

    rust::Vec<Bindings::RustyCharacteristicWrapper> characteristics() const;

  private:
    // NOTE: All internal properties need to be handled as pointers,
    // allowing the calls to RustyService to always be const.
    // This might require us to store pointers to pointers, so it's
    // important to be careful when handling these.
    std::shared_ptr<SimpleBLE::Service> _internal;
};

class RustyCharacteristic {
  public:
    RustyCharacteristic() = default;
    virtual ~RustyCharacteristic() = default;

    RustyCharacteristic(SimpleBLE::Characteristic characteristic)
        : _internal(new SimpleBLE::Characteristic(std::move(characteristic))) {}

    bool initialized() const { return _internal->initialized(); }
    rust::String uuid() const { return rust::String(_internal->uuid()); }

    rust::Vec<Bindings::RustyDescriptorWrapper> descriptors() const;
    rust::Vec<rust::String> capabilities() const;

    bool can_read() const { return _internal->can_read(); }
    bool can_write_request() const { return _internal->can_write_request(); }
    bool can_write_command() const { return _internal->can_write_command(); }
    bool can_notify() const { return _internal->can_notify(); }
    bool can_indicate() const { return _internal->can_indicate(); }

  private:
    // NOTE: All internal properties need to be handled as pointers,
    // allowing the calls to RustyCharacteristic to always be const.
    // This might require us to store pointers to pointers, so it's
    // important to be careful when handling these.
    std::shared_ptr<SimpleBLE::Characteristic> _internal;
};

class RustyDescriptor {
  public:
    RustyDescriptor() = default;
    virtual ~RustyDescriptor() = default;

    RustyDescriptor(SimpleBLE::Descriptor descriptor) : _internal(new SimpleBLE::Descriptor(std::move(descriptor))) {}

    bool initialized() const { return _internal->initialized(); }
    rust::String uuid() const { return rust::String(_internal->uuid()); }

  private:
    // NOTE: All internal properties need to be handled as pointers,
    // allowing the calls to RustyDescriptor to always be const.
    // This might require us to store pointers to pointers, so it's
    // important to be careful when handling these.
    std::shared_ptr<SimpleBLE::Descriptor> _internal;
};

};  // namespace Bindings
