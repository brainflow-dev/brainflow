#import "PeripheralBase.h"
#import "PeripheralBaseMacOS.h"

#include <iostream>

using namespace SimpleBLE;

PeripheralBase::PeripheralBase(void* opaque_peripheral, void* opaque_adapter, advertising_data_t advertising_data) {
    this->opaque_internal_ = [[PeripheralBaseMacOS alloc] init:(CBPeripheral*)opaque_peripheral
                                                centralManager:(CBCentralManager*)opaque_adapter];
    this->is_connectable_ = advertising_data.connectable;
    this->manual_disconnect_triggered_ = false;
    this->manufacturer_data_ = advertising_data.manufacturer_data;
}

PeripheralBase::~PeripheralBase() {}

std::string PeripheralBase::identifier() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;
    return std::string([[internal identifier] UTF8String]);
}

BluetoothAddress PeripheralBase::address() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;
    return std::string([[internal address] UTF8String]);
}

void PeripheralBase::connect() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;
    [internal connect];
    if (callback_on_connected_) {
        callback_on_connected_();
    }
}

void PeripheralBase::disconnect() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;

    manual_disconnect_triggered_ = true;
    [internal disconnect];
    if (callback_on_disconnected_) {
        callback_on_disconnected_();
    }
    manual_disconnect_triggered_ = false;
}

bool PeripheralBase::is_connected() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;
    return [internal isConnected];
}

bool PeripheralBase::is_connectable() { return is_connectable_; }

std::vector<BluetoothService> PeripheralBase::services() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;
    return [internal getServices];
}

std::map<uint16_t, ByteArray> PeripheralBase::manufacturer_data() {
    return manufacturer_data_;
}

ByteArray PeripheralBase::read(BluetoothUUID service, BluetoothUUID characteristic) {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];

    return [internal read:service_uuid characteristic_uuid:characteristic_uuid];
}

void PeripheralBase::write_request(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data) {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    NSData* payload = [NSData dataWithBytes:(void*)data.c_str() length:data.size()];

    [internal writeRequest:service_uuid characteristic_uuid:characteristic_uuid payload:payload];
}

void PeripheralBase::write_command(BluetoothUUID service, BluetoothUUID characteristic, ByteArray data) {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    NSData* payload = [NSData dataWithBytes:(void*)data.c_str() length:data.size()];

    [internal writeCommand:service_uuid characteristic_uuid:characteristic_uuid payload:payload];
}

void PeripheralBase::notify(BluetoothUUID service, BluetoothUUID characteristic, std::function<void(ByteArray payload)> callback) {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    [internal notify:service_uuid characteristic_uuid:characteristic_uuid callback:callback];
}

void PeripheralBase::indicate(BluetoothUUID service, BluetoothUUID characteristic, std::function<void(ByteArray payload)> callback) {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    [internal indicate:service_uuid characteristic_uuid:characteristic_uuid callback:callback];
}

void PeripheralBase::unsubscribe(BluetoothUUID service, BluetoothUUID characteristic) {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;

    NSString* service_uuid = [NSString stringWithCString:service.c_str() encoding:NSString.defaultCStringEncoding];
    NSString* characteristic_uuid = [NSString stringWithCString:characteristic.c_str() encoding:NSString.defaultCStringEncoding];
    [internal unsubscribe:service_uuid characteristic_uuid:characteristic_uuid];
}

void PeripheralBase::set_callback_on_connected(std::function<void()> on_connected) { callback_on_connected_ = on_connected; }

void PeripheralBase::set_callback_on_disconnected(std::function<void()> on_disconnected) { callback_on_disconnected_ = on_disconnected; }

void PeripheralBase::delegate_did_connect() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;
    [internal delegateDidConnect];
}

void PeripheralBase::delegate_did_disconnect() {
    PeripheralBaseMacOS* internal = (PeripheralBaseMacOS*)opaque_internal_;
    [internal delegateDidDisconnect];

    // If the user manually disconnects the peripheral, don't call the callback at this point.
    if (callback_on_disconnected_ && !manual_disconnect_triggered_) {
        callback_on_disconnected_();
    }
}
