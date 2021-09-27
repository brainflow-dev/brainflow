#import "AdapterBase.h"
#import "AdapterBaseMacOS.h"
#import "PeripheralBase.h"
#import "PeripheralBuilder.h"

#include <chrono>
#include <thread>

using namespace SimpleBLE;

AdapterBase::AdapterBase() { opaque_internal_ = [[AdapterBaseMacOS alloc] init:this]; }

AdapterBase::~AdapterBase() {
    // Explicitly release the opaque pointer, as automatic
    // reference counting does not work appropriately.
    [(id)opaque_internal_ release];
}

std::vector<std::shared_ptr<AdapterBase> > AdapterBase::get_adapters() {
    // There doesn't seem to be a mechanism with Apple devices that openly
    // exposes more than the default Bluetooth device.
    // For this reason, the MacOS implementation of SimpleBLE will only
    // consider that single case, at least until better alternatives come up.

    std::vector<std::shared_ptr<AdapterBase> > adapter_list;
    adapter_list.push_back(std::make_shared<AdapterBase>());
    return adapter_list;
}

std::string AdapterBase::identifier() { return "Default Adapter"; }

BluetoothAddress AdapterBase::address() { return "00:00:00:00:00:00"; }

void AdapterBase::scan_start() {
    AdapterBaseMacOS* internal = (AdapterBaseMacOS*)opaque_internal_;
    [internal scanStart];

    if (callback_on_scan_start_) {
        callback_on_scan_start_();
    }
}

void AdapterBase::scan_stop() {
    AdapterBaseMacOS* internal = (AdapterBaseMacOS*)opaque_internal_;
    [internal scanStop];
    if (callback_on_scan_stop_) {
        callback_on_scan_stop_();
    }
}

void AdapterBase::scan_for(int timeout_ms) {
    this->scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    this->scan_stop();
}

bool AdapterBase::scan_is_active() {
    AdapterBaseMacOS* internal = (AdapterBaseMacOS*)opaque_internal_;
    return [internal scanIsActive];
}

std::vector<Peripheral> AdapterBase::scan_get_results() {
    std::vector<Peripheral> peripherals;
    for (auto& [opaque_peripheral, base_peripheral] : this->peripherals_) {
        PeripheralBuilder peripheral_builder(base_peripheral);
        peripherals.push_back(peripheral_builder);
    }
    return peripherals;
}

void AdapterBase::set_callback_on_scan_start(std::function<void()> on_scan_start) { callback_on_scan_start_ = on_scan_start; }

void AdapterBase::set_callback_on_scan_stop(std::function<void()> on_scan_stop) { callback_on_scan_stop_ = on_scan_stop; }

void AdapterBase::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    callback_on_scan_updated_ = on_scan_updated;
}
void AdapterBase::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) { callback_on_scan_found_ = on_scan_found; }

// Delegate methods passed for AdapterBaseMacOS

void AdapterBase::delegate_did_discover_peripheral(void* opaque_peripheral, void* opaque_adapter, advertising_data_t advertising_data) {
    if (this->peripherals_.count(opaque_peripheral) == 0) {
        // Create a new PeripheralBase object
        std::shared_ptr<PeripheralBase> base_peripheral = std::make_shared<PeripheralBase>(opaque_peripheral, opaque_adapter,
                                                                                           advertising_data);

        // Store it in our table of seem peripherals
        this->peripherals_.insert(std::make_pair(opaque_peripheral, base_peripheral));

        // Convert the base object into an external-facing Peripheral object
        PeripheralBuilder peripheral_builder(base_peripheral);
        if (this->callback_on_scan_found_) {
            this->callback_on_scan_found_(peripheral_builder);
        }
    } else {
        // Load the existing PeripheralBase object
        std::shared_ptr<PeripheralBase> base_peripheral = this->peripherals_.at(opaque_peripheral);

        // Convert the base object into an external-facing Peripheral object
        PeripheralBuilder peripheral_builder(base_peripheral);
        if (this->callback_on_scan_updated_) {
            this->callback_on_scan_updated_(peripheral_builder);
        }
    }
}

void AdapterBase::delegate_did_connect_peripheral(void* opaque_peripheral) {
    if (this->peripherals_.count(opaque_peripheral) == 0) {
        throw Exception::InvalidReference();
    }

    // Load the existing PeripheralBase object
    std::shared_ptr<PeripheralBase> base_peripheral = this->peripherals_.at(opaque_peripheral);
    base_peripheral->delegate_did_connect();
}

void AdapterBase::delegate_did_disconnect_peripheral(void* opaque_peripheral) {
    if (this->peripherals_.count(opaque_peripheral) == 0) {
        throw Exception::InvalidReference();
    }

    // Load the existing PeripheralBase object
    std::shared_ptr<PeripheralBase> base_peripheral = this->peripherals_.at(opaque_peripheral);
    base_peripheral->delegate_did_disconnect();
}
