#include <thread>

#include <simpleble/Peripheral.h>

#include "AdapterLinux.h"
#include "BuildVec.h"
#include "BuilderBase.h"
#include "CommonUtils.h"
#include "PeripheralLinux.h"

using namespace SimpleBLE;

bool AdapterLinux::bluetooth_enabled() { return adapter_->powered(); }

AdapterLinux::AdapterLinux(std::shared_ptr<SimpleBluez::Adapter> adapter) : adapter_(adapter) {}

AdapterLinux::~AdapterLinux() { adapter_->clear_on_device_updated(); }

void* AdapterLinux::underlying() const { return adapter_.get(); }

std::string AdapterLinux::identifier() { return adapter_->identifier(); }

BluetoothAddress AdapterLinux::address() { return adapter_->address(); }

void AdapterLinux::power_on() { adapter_->powered(true); }

void AdapterLinux::power_off() { adapter_->powered(false); }

bool AdapterLinux::is_powered() { return adapter_->powered(); }

void AdapterLinux::scan_start() {
    seen_peripherals_.clear();

    adapter_->set_on_device_updated([this](std::shared_ptr<SimpleBluez::Device> device) {
        if (!this->is_scanning_) {
            return;
        }

        if (this->peripherals_.count(device->address()) == 0) {
            // If the incoming peripheral has never been seen before, create and save a reference to it.
            auto base_peripheral = std::make_shared<PeripheralLinux>(device, this->adapter_);
            this->peripherals_.insert(std::make_pair(device->address(), base_peripheral));
        }

        // Update the received advertising data.
        auto peripheral = this->peripherals_.at(device->address());

        // Check if the device has been seen before, to forward the correct call to the user.
        if (this->seen_peripherals_.count(device->address()) == 0) {
            // Store it in our table of seen peripherals
            this->seen_peripherals_.insert(std::make_pair(device->address(), peripheral));
            SAFE_CALLBACK_CALL(this->_callback_on_scan_found, Factory::build(peripheral));
        } else {
            SAFE_CALLBACK_CALL(this->_callback_on_scan_updated, Factory::build(peripheral));
        }
    });

    // Start scanning and notify the user.
    adapter_->discovery_start();

    // TODO: Does a discovery filter need to be set?

    SAFE_CALLBACK_CALL(this->_callback_on_scan_start);
    is_scanning_ = true;
}

void AdapterLinux::scan_stop() {
    adapter_->discovery_stop();
    is_scanning_ = false;
    SAFE_CALLBACK_CALL(this->_callback_on_scan_stop);

    // Important: Bluez might continue scanning if another process is also requesting
    // scanning from the adapter. The use of the is_scanning_ flag is to prevent
    // any scan updates to reach the user when not expected.
}

void AdapterLinux::scan_for(int timeout_ms) {
    scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    scan_stop();
}

bool AdapterLinux::scan_is_active() { return is_scanning_ && adapter_->discovering(); }

SharedPtrVector<PeripheralBase> AdapterLinux::scan_get_results() { return Util::values(seen_peripherals_); }

SharedPtrVector<PeripheralBase> AdapterLinux::get_paired_peripherals() {
    SharedPtrVector<PeripheralBase> peripherals;

    auto paired_list = adapter_->device_paired_get();
    for (auto& device : paired_list) {
        peripherals.push_back(std::make_shared<PeripheralLinux>(device, this->adapter_));
    }

    return peripherals;
}
