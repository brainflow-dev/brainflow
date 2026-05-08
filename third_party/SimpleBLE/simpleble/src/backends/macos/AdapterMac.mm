#include <simpleble/Peripheral.h>

#import "AdapterBaseMacOS.h"
#import "AdapterMac.h"
#import "BuilderBase.h"
#import "CommonUtils.h"
#import "PeripheralMac.h"

#include <fmt/core.h>

#include <chrono>
#include <memory>
#include <thread>

using namespace SimpleBLE;

AdapterMac::AdapterMac() {
    // Cast the Objective-C++ object using __bridge_retained, which will signal ARC to increase
    // the reference count. This means that AdapterMac will be responsible for releasing the
    // Objective-C++ object in the destructor.
    opaque_internal_ = (__bridge_retained void*)[[AdapterBaseMacOS alloc] init:this];
}

AdapterMac::~AdapterMac() {
    // Cast the opaque pointer back to the Objective-C++ object and release it.
    // This will signal ARC to decrease the reference count.
    // NOTE: This is equivalent to calling [opaque_internal_ release] in Objective-C++.
    AdapterBaseMacOS* internal = (__bridge_transfer AdapterBaseMacOS*)opaque_internal_;
    internal = nil;
}

bool AdapterMac::bluetooth_enabled() {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;
    return [internal isBluetoothEnabled];
}

void* AdapterMac::underlying() const {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;

    return [internal underlying];
}

std::string AdapterMac::identifier() { return fmt::format("Default Adapter [{}]", this->address()); }

BluetoothAddress AdapterMac::address() const {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;

    return [[internal address] UTF8String];
}

void AdapterMac::power_on() {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;
    [internal powerOn];
}

void AdapterMac::power_off() {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;
    [internal powerOff];
}

bool AdapterMac::is_powered() {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;
    return [internal isPowered];
}

BluetoothAddress AdapterMac::address() { return const_cast<const AdapterMac*>(this)->address(); }

void AdapterMac::scan_start() {
    this->seen_peripherals_.clear();

    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;
    [internal scanStart];

    SAFE_CALLBACK_CALL(this->_callback_on_scan_start);
}

void AdapterMac::scan_stop() {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;
    [internal scanStop];

    SAFE_CALLBACK_CALL(this->_callback_on_scan_stop);
}

void AdapterMac::scan_for(int timeout_ms) {
    this->scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    this->scan_stop();
}

bool AdapterMac::scan_is_active() {
    AdapterBaseMacOS* internal = (__bridge AdapterBaseMacOS*)opaque_internal_;
    return [internal scanIsActive];
}

SharedPtrVector<PeripheralBase> AdapterMac::scan_get_results() {
    SharedPtrVector<PeripheralBase> peripherals = Util::values(seen_peripherals_);
    return peripherals;
}

SharedPtrVector<PeripheralBase> AdapterMac::get_paired_peripherals() { return {}; }

// Delegate methods passed for AdapterBaseMacOS

void AdapterMac::delegate_did_discover_peripheral(void* opaque_peripheral, void* opaque_adapter, advertising_data_t advertising_data) {
    if (this->peripherals_.count(opaque_peripheral) == 0) {
        // If the incoming peripheral has never been seen before, create and save a reference to it.
        auto base_peripheral = std::make_shared<PeripheralMac>(opaque_peripheral, opaque_adapter, advertising_data);
        this->peripherals_.insert(std::make_pair(opaque_peripheral, base_peripheral));
    }

    // Update the received advertising data.
    auto base_peripheral = this->peripherals_.at(opaque_peripheral);
    base_peripheral->update_advertising_data(advertising_data);

    // Convert the base object into an external-facing Peripheral object
    Peripheral peripheral = Factory::build(base_peripheral);

    // Check if the device has been seen before, to forward the correct call to the user.
    if (this->seen_peripherals_.count(opaque_peripheral) == 0) {
        // Store it in our table of seen peripherals
        this->seen_peripherals_.insert(std::make_pair(opaque_peripheral, base_peripheral));
        SAFE_CALLBACK_CALL(this->_callback_on_scan_found, peripheral);
    } else {
        SAFE_CALLBACK_CALL(this->_callback_on_scan_updated, peripheral);
    }
}

void AdapterMac::delegate_did_connect_peripheral(void* opaque_peripheral) {
    if (this->peripherals_.count(opaque_peripheral) == 0) {
        throw Exception::InvalidReference();
    }

    // Load the existing PeripheralBase object
    std::shared_ptr<PeripheralMac> base_peripheral = this->peripherals_.at(opaque_peripheral);
    base_peripheral->delegate_did_connect();
}

void AdapterMac::delegate_did_fail_to_connect_peripheral(void* opaque_peripheral, void* opaque_error) {
    if (this->peripherals_.count(opaque_peripheral) == 0) {
        throw Exception::InvalidReference();
    }

    // Load the existing PeripheralBase object
    std::shared_ptr<PeripheralMac> base_peripheral = this->peripherals_.at(opaque_peripheral);
    base_peripheral->delegate_did_fail_to_connect(opaque_error);
}

void AdapterMac::delegate_did_disconnect_peripheral(void* opaque_peripheral, void* opaque_error) {
    if (this->peripherals_.count(opaque_peripheral) == 0) {
        throw Exception::InvalidReference();
    }

    // Load the existing PeripheralBase object
    std::shared_ptr<PeripheralMac> base_peripheral = this->peripherals_.at(opaque_peripheral);
    base_peripheral->delegate_did_disconnect(opaque_error);
}
