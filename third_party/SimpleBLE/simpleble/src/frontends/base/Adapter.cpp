#include <simpleble/Adapter.h>

#include "AdapterBase.h"
#include "AdapterBuilder.h"
#include "LoggingInternal.h"

using namespace SimpleBLE;

std::vector<Adapter> Adapter::get_adapters() {
    std::vector<Adapter> available_adapters;
    auto internal_adapters = AdapterBase::get_adapters();

    for (auto& internal_adapter : internal_adapters) {
        AdapterBuilder adapter(internal_adapter);
        available_adapters.push_back(adapter);
    }

    return available_adapters;
}

bool Adapter::bluetooth_enabled() noexcept {
    try {
        return AdapterBase::bluetooth_enabled();
    } catch (const std::exception& e) {
        SIMPLEBLE_LOG_ERROR(fmt::format("Failed to check if bluetooth is enabled: {}", e.what()));
        return false;
    } catch (...) {
        SIMPLEBLE_LOG_ERROR("Failed to check if bluetooth is enabled: Unknown error");
        return false;
    }
}

bool Adapter::initialized() const { return internal_ != nullptr; }

void* Adapter::underlying() const {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->underlying();
}

std::string Adapter::identifier() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->identifier();
}

BluetoothAddress Adapter::address() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->address();
}

void Adapter::scan_start() {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->scan_start();
}

void Adapter::scan_stop() {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->scan_stop();
}

void Adapter::scan_for(int timeout_ms) {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->scan_for(timeout_ms);
}

bool Adapter::scan_is_active() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->scan_is_active();
}

std::vector<Peripheral> Adapter::scan_get_results() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->scan_get_results();
}

std::vector<Peripheral> Adapter::get_paired_peripherals() {
    if (!initialized()) throw Exception::NotInitialized();

    return internal_->get_paired_peripherals();
}

void Adapter::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->set_callback_on_scan_start(std::move(on_scan_start));
}

void Adapter::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->set_callback_on_scan_stop(std::move(on_scan_stop));
}

void Adapter::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->set_callback_on_scan_updated(std::move(on_scan_updated));
}

void Adapter::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    if (!initialized()) throw Exception::NotInitialized();

    internal_->set_callback_on_scan_found(std::move(on_scan_found));
}
