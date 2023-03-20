#include "AdapterBase.h"
#include "Bluez.h"
#include "CommonUtils.h"
#include "PeripheralBase.h"
#include "PeripheralBuilder.h"

using namespace SimpleBLE;

std::vector<std::shared_ptr<AdapterBase>> AdapterBase::get_adapters() {
    std::vector<std::shared_ptr<AdapterBase>> adapter_list;
    auto internal_adapters = Bluez::get()->bluez.get_adapters();
    for (auto& adapter : internal_adapters) {
        adapter_list.push_back(std::make_shared<AdapterBase>(adapter));
    }
    return adapter_list;
}

bool AdapterBase::bluetooth_enabled() {
    bool enabled = false;

    auto internal_adapters = Bluez::get()->bluez.get_adapters();
    for (auto& adapter : internal_adapters) {
        if (adapter->powered()) {
            enabled = true;
            break;
        }
    }

    return enabled;
}

AdapterBase::AdapterBase(std::shared_ptr<SimpleBluez::Adapter> adapter) : adapter_(adapter) {}

AdapterBase::~AdapterBase() { adapter_->clear_on_device_updated(); }

void* AdapterBase::underlying() const { return adapter_.get(); }

std::string AdapterBase::identifier() { return adapter_->identifier(); }

BluetoothAddress AdapterBase::address() { return adapter_->address(); }

void AdapterBase::scan_start() {
    seen_peripherals_.clear();

    adapter_->set_on_device_updated([this](std::shared_ptr<SimpleBluez::Device> device) {
        if (!this->is_scanning_) {
            return;
        }

        if (this->peripherals_.count(device->address()) == 0) {
            // If the incoming peripheral has never been seen before, create and save a reference to it.
            auto base_peripheral = std::make_shared<PeripheralBase>(device, this->adapter_);
            this->peripherals_.insert(std::make_pair(device->address(), base_peripheral));
        }

        // Update the received advertising data.
        auto base_peripheral = this->peripherals_.at(device->address());

        // Convert the base object into an external-facing Peripheral object
        PeripheralBuilder peripheral_builder(base_peripheral);

        // Check if the device has been seen before, to forward the correct call to the user.
        if (this->seen_peripherals_.count(device->address()) == 0) {
            // Store it in our table of seen peripherals
            this->seen_peripherals_.insert(std::make_pair(device->address(), base_peripheral));
            SAFE_CALLBACK_CALL(this->callback_on_scan_found_, peripheral_builder);
        } else {
            SAFE_CALLBACK_CALL(this->callback_on_scan_updated_, peripheral_builder);
        }
    });

    // Start scanning and notify the user.
    adapter_->discovery_start();
    SAFE_CALLBACK_CALL(this->callback_on_scan_start_);
    is_scanning_ = true;
}

void AdapterBase::scan_stop() {
    adapter_->discovery_stop();
    is_scanning_ = false;
    SAFE_CALLBACK_CALL(this->callback_on_scan_stop_);

    // Important: Bluez might continue scanning if another process is also requesting
    // scanning from the adapter. The use of the is_scanning_ flag is to prevent
    // any scan updates to reach the user when not expected.
}

void AdapterBase::scan_for(int timeout_ms) {
    scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    scan_stop();
}

bool AdapterBase::scan_is_active() { return is_scanning_ && adapter_->discovering(); }

std::vector<Peripheral> AdapterBase::scan_get_results() {
    std::vector<Peripheral> peripherals;
    for (auto& [address, peripheral] : this->seen_peripherals_) {
        PeripheralBuilder peripheral_builder(peripheral);
        peripherals.push_back(peripheral_builder);
    }
    return peripherals;
}

std::vector<Peripheral> AdapterBase::get_paired_peripherals() {
    std::vector<Peripheral> peripherals;

    auto paired_list = adapter_->device_paired_get();
    for (auto& device : paired_list) {
        PeripheralBuilder peripheral_builder(std::make_shared<PeripheralBase>(device, this->adapter_));
        peripherals.push_back(peripheral_builder);
    }

    return peripherals;
}

void AdapterBase::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    if (on_scan_start) {
        callback_on_scan_start_.load(std::move(on_scan_start));
    } else {
        callback_on_scan_start_.unload();
    }
}

void AdapterBase::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    if (on_scan_stop) {
        callback_on_scan_stop_.load(std::move(on_scan_stop));
    } else {
        callback_on_scan_stop_.unload();
    }
}

void AdapterBase::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    if (on_scan_updated) {
        callback_on_scan_updated_.load(std::move(on_scan_updated));
    } else {
        callback_on_scan_updated_.unload();
    }
}

void AdapterBase::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    if (on_scan_found) {
        callback_on_scan_found_.load(std::move(on_scan_found));
    } else {
        callback_on_scan_found_.unload();
    }
}
