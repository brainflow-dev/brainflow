#include "AdapterBase.h"
#include "Bluez.h"
#include "PeripheralBase.h"
#include "PeripheralBuilder.h"

#include <iostream>

using namespace SimpleBLE;

std::vector<std::shared_ptr<AdapterBase>> AdapterBase::get_adapters() {
    std::vector<std::shared_ptr<AdapterBase>> adapter_list;
    auto internal_adapters = Bluez::get()->bluez_service.get_all_adapters();
    for (auto& adapter : internal_adapters) {
        adapter_list.push_back(std::make_shared<AdapterBase>(adapter));
    }
    return adapter_list;
}

AdapterBase::AdapterBase(std::shared_ptr<BluezAdapter> adapter) { adapter_ = adapter; }

AdapterBase::~AdapterBase() {}

std::string AdapterBase::identifier() {
    auto adapter = adapter_.lock();
    if (adapter) {
        return adapter->get_identifier();
    } else {
        throw Exception::InvalidReference();
    }
}

BluetoothAddress AdapterBase::address() {
    auto adapter = adapter_.lock();
    if (adapter) {
        return adapter->Address();
    } else {
        throw Exception::InvalidReference();
    }
}

void AdapterBase::scan_start() {
    auto adapter = adapter_.lock();
    if (adapter) {
        adapter->discovery_filter_transport_set("le");
        this->seen_devices_.clear();
        adapter->OnDeviceUpdated = [&](std::shared_ptr<BluezDevice> device) {
            PeripheralBuilder peripheral_builder(std::make_shared<PeripheralBase>(device));

            if (this->seen_devices_.count(peripheral_builder.address()) == 0) {
                this->seen_devices_.insert(std::make_pair<>(peripheral_builder.address(), peripheral_builder));
                if (this->callback_on_scan_found_) {
                    this->callback_on_scan_found_(peripheral_builder);
                }
            } else {
                if (this->callback_on_scan_updated_) {
                    this->callback_on_scan_updated_(peripheral_builder);
                }
            }
        };

        // Start scanning and notify the user.
        adapter->StartDiscovery();
        if (callback_on_scan_start_) {
            callback_on_scan_start_();
        }
    } else {
        throw Exception::InvalidReference();
    }
}

void AdapterBase::scan_stop() {
    auto adapter = adapter_.lock();
    if (adapter) {
        adapter->StopDiscovery();
        // Due to the fact that Bluez takes some time to process the command
        // and for SimpleDBus to flush the queue, wait until the driver
        // has acknowledged that it is no longer discovering.
        while (adapter->Property_Discovering()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (callback_on_scan_stop_) {
            callback_on_scan_stop_();
        }
    } else {
        throw Exception::InvalidReference();
    }
}

void AdapterBase::scan_for(int timeout_ms) {
    scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    scan_stop();
}

bool AdapterBase::scan_is_active() {
    auto adapter = adapter_.lock();
    if (adapter) {
        return adapter->Property_Discovering();
    } else {
        throw Exception::InvalidReference();
    }
}

std::vector<Peripheral> AdapterBase::scan_get_results() {
    std::vector<Peripheral> peripherals;
    for (auto& [address, peripheral] : this->seen_devices_) {
        peripherals.push_back(peripheral);
    }
    return peripherals;
}

void AdapterBase::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    callback_on_scan_start_ = on_scan_start;
}
void AdapterBase::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    callback_on_scan_stop_ = on_scan_stop;
}
void AdapterBase::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    callback_on_scan_updated_ = on_scan_updated;
}
void AdapterBase::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    callback_on_scan_found_ = on_scan_found;
}
