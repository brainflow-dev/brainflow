#import "PeripheralBase.h"
#import "PeripheralBaseMacOS.h"
#import "ServiceBuilder.h"

#import "CommonUtils.h"

#include <iostream>

using namespace SimpleBLE;

PeripheralBase::PeripheralBase(void* opaque_peripheral, void* opaque_adapter, advertising_data_t advertising_data) {
    CBCentralManager* central_manager = (__bridge CBCentralManager*)opaque_adapter;
    CBPeripheral* peripheral = (__bridge CBPeripheral*)opaque_peripheral;

    // Cast the Objective-C++ object using __bridge_retained, which will signal ARC to increase
    // the reference count. This means that AdapterBase will be responsible for releasing the
    // Objective-C++ object in the destructor.
    opaque_internal_ = (__bridge_retained void*)[[PeripheralBaseMacOS alloc] init:peripheral centralManager:central_manager];

    is_connectable_ = advertising_data.connectable;
    manual_disconnect_triggered_ = false;
    manufacturer_data_ = advertising_data.manufacturer_data;
    service_data_ = advertising_data.service_data;
    rssi_ = advertising_data.rssi;
    tx_power_ = advertising_data.tx_power;
}

PeripheralBase::~PeripheralBase() {
    // Cast the opaque pointer back to the Objective-C++ object and release it.
    // This will signal ARC to decrease the reference count.
    // NOTE: This is equivalent to calling [opaque_internal_ release] in Objective-C++.
    PeripheralBaseMacOS* internal = (__bridge_transfer PeripheralBaseMacOS*)opaque_internal_;
    internal = nil;
}

void* PeripheralBase::underlying() const {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    return [internal underlying];
}

std::string PeripheralBase::identifier() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    return std::string([[internal identifier] UTF8String]);
}

BluetoothAddress PeripheralBase::address() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    return std::string([[internal address] UTF8String]);
}

BluetoothAddressType PeripheralBase::address_type() {
    return BluetoothAddressType::UNSPECIFIED;
}

int16_t PeripheralBase::rssi() { return rssi_; }

int16_t PeripheralBase::tx_power() { return tx_power_; }

uint16_t PeripheralBase::mtu() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    return [internal mtu];
}

void PeripheralBase::update_advertising_data(advertising_data_t advertising_data) {
    is_connectable_ = advertising_data.connectable;
    manufacturer_data_ = advertising_data.manufacturer_data;
    rssi_ = advertising_data.rssi;
    tx_power_ = advertising_data.tx_power;

    advertising_data.service_data.merge(service_data_);
    service_data_ = advertising_data.service_data;
}

void PeripheralBase::connect() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    [internal connect];

    SAFE_CALLBACK_CALL(this->callback_on_connected_);
}

void PeripheralBase::disconnect() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    manual_disconnect_triggered_ = true;
    [internal disconnect];

    SAFE_CALLBACK_CALL(this->callback_on_disconnected_);

    manual_disconnect_triggered_ = false;
}

bool PeripheralBase::is_connected() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    return [internal isConnected];
}

bool PeripheralBase::is_connectable() { return is_connectable_; }

bool PeripheralBase::is_paired() { throw Exception::OperationNotSupported(); }

void PeripheralBase::unpair() { throw Exception::OperationNotSupported(); }

std::vector<Service> PeripheralBase::services() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    return [internal getServices];
}

std::vector<Service> PeripheralBase::advertised_services() {
    std::vector<Service> service_list;
    for (auto& [service_uuid, data] : service_data_) {
        service_list.push_back(ServiceBuilder(service_uuid, data));
    }

    return service_list;
}

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() { return manufacturer_data_; }

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];

    return [internal read:service_uuid characteristic_uuid:characteristic_uuid];
}

void PeripheralBase::write_request(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    NSData* payload = [NSData dataWithBytes:(void*)data.c_str() length:data.size()];

    [internal writeRequest:service_uuid characteristic_uuid:characteristic_uuid payload:payload];
}

void PeripheralBase::write_command(BluetoothUUID const& service, BluetoothUUID const& characteristic, ByteArray const& data) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    NSData* payload = [NSData dataWithBytes:(void*)data.c_str() length:data.size()];

    [internal writeCommand:service_uuid characteristic_uuid:characteristic_uuid payload:payload];
}

void PeripheralBase::notify(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                            std::function<void(ByteArray payload)> callback) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    [internal notify:service_uuid characteristic_uuid:characteristic_uuid callback:callback];
}

void PeripheralBase::indicate(BluetoothUUID const& service, BluetoothUUID const& characteristic,
                              std::function<void(ByteArray payload)> callback) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    [internal indicate:service_uuid characteristic_uuid:characteristic_uuid callback:callback];
}

void PeripheralBase::unsubscribe(BluetoothUUID const& service, BluetoothUUID const& characteristic) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    [internal unsubscribe:service_uuid characteristic_uuid:characteristic_uuid];
}

ByteArray PeripheralBase::read(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* descriptor_uuid = [NSString stringWithCString:descriptor.c_str() encoding:NSString.defaultCStringEncoding];

    return [internal read:service_uuid characteristic_uuid:characteristic_uuid descriptor_uuid:descriptor_uuid];
}

void PeripheralBase::write(BluetoothUUID const& service, BluetoothUUID const& characteristic, BluetoothUUID const& descriptor,
                           ByteArray const& data) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* descriptor_uuid = [NSString stringWithCString:descriptor.c_str() encoding:NSString.defaultCStringEncoding];
    NSData* payload = [NSData dataWithBytes:(void*)data.c_str() length:data.size()];

    [internal write:service_uuid characteristic_uuid:characteristic_uuid descriptor_uuid:descriptor_uuid payload:payload];
}

void PeripheralBase::set_callback_on_connected(std::function<void()> on_connected) {
    if (on_connected) {
        callback_on_connected_.load(std::move(on_connected));
    } else {
        callback_on_connected_.unload();
    }
}

void PeripheralBase::set_callback_on_disconnected(std::function<void()> on_disconnected) {
    if (on_disconnected) {
        callback_on_disconnected_.load(std::move(on_disconnected));
    } else {
        callback_on_disconnected_.unload();
    }
}

void PeripheralBase::delegate_did_connect() {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    [internal delegateDidConnect];
}

void PeripheralBase::delegate_did_fail_to_connect(void* opaque_error) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    NSError* error = (__bridge NSError*)opaque_error;
    [internal delegateDidFailToConnect : error];
}

void PeripheralBase::delegate_did_disconnect(void* opaque_error) {
    PeripheralBaseMacOS* internal = (__bridge PeripheralBaseMacOS*)opaque_internal_;
    NSError* error = (__bridge NSError*)opaque_error;
    [internal delegateDidDisconnect : error];

    // If the user manually disconnects the peripheral, don't call the callback at this point.
    if (callback_on_disconnected_ && !manual_disconnect_triggered_) {
        callback_on_disconnected_();
    }
}
