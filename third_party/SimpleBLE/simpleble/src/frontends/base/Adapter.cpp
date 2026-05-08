#include <simpleble/Adapter.h>
#include "Backend.h"

#include "BuildVec.h"
#include "LoggingInternal.h"
#include "backends/common/AdapterBase.h"

using namespace SimpleBLE;

std::vector<Adapter> Adapter::get_adapters() {
    std::vector<Adapter> adapter_list;
    for (auto& backend : Backend::get_backends()) {
        for (auto& adapter : backend.get_adapters()) {
            adapter_list.push_back(adapter);
        }
    }

    return adapter_list;
}

// TODO: this should be the implementation of the per-backend bluetooth_enabled() function
// bool Adapter::bluetooth_enabled() { return (*this)->bluetooth_enabled(); }

bool Adapter::bluetooth_enabled() { return get_enabled_backend().bluetooth_enabled(); }

bool Adapter::initialized() const { return internal_ != nullptr; }

void* Adapter::underlying() const { return (*this)->underlying(); }

/**
 * Return the adapter implementation if it is initialized.
 *
 * @throws SimpleBLE::NotInitialized if the adapter is not initialized.
 */
AdapterBase* Adapter::operator->() {
    if (!internal_) {
        throw Exception::NotInitialized();
    }
    return internal_.get();
}

const AdapterBase* Adapter::operator->() const {
    if (!internal_) {
        throw Exception::NotInitialized();
    }
    return internal_.get();
}

std::string Adapter::identifier() { return (*this)->identifier(); }

BluetoothAddress Adapter::address() { return (*this)->address(); }

void Adapter::power_on() { (*this)->power_on(); }

void Adapter::power_off() { (*this)->power_off(); }

bool Adapter::is_powered() { return (*this)->is_powered(); }

void Adapter::set_callback_on_power_on(std::function<void()> on_power_on) {
    (*this)->set_callback_on_power_on(std::move(on_power_on));
}

void Adapter::set_callback_on_power_off(std::function<void()> on_power_off) {
    (*this)->set_callback_on_power_off(std::move(on_power_off));
}

void Adapter::scan_start() {
    if (!bluetooth_enabled()) {
        SIMPLEBLE_LOG_WARN(fmt::format("Bluetooth is not enabled."));
        return;
    }
    (*this)->scan_start();
}

void Adapter::scan_stop() {
    if (!bluetooth_enabled()) {
        SIMPLEBLE_LOG_WARN(fmt::format("Bluetooth is not enabled."));
        return;
    }
    (*this)->scan_stop();
}

void Adapter::scan_for(int timeout_ms) {
    if (!bluetooth_enabled()) {
        SIMPLEBLE_LOG_WARN(fmt::format("Bluetooth is not enabled."));
        return;
    }
    (*this)->scan_for(timeout_ms);
}

bool Adapter::scan_is_active() { return (*this)->scan_is_active(); }

std::vector<Peripheral> Adapter::scan_get_results() { return Factory::vector((*this)->scan_get_results()); }

std::vector<Peripheral> Adapter::get_paired_peripherals() { return Factory::vector((*this)->get_paired_peripherals()); }

std::vector<Peripheral> Adapter::get_connected_peripherals() { return Factory::vector((*this)->get_connected_peripherals()); }

void Adapter::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    (*this)->set_callback_on_scan_start(std::move(on_scan_start));
}

void Adapter::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    (*this)->set_callback_on_scan_stop(std::move(on_scan_stop));
}

void Adapter::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    (*this)->set_callback_on_scan_updated(std::move(on_scan_updated));
}

void Adapter::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    (*this)->set_callback_on_scan_found(std::move(on_scan_found));
}
