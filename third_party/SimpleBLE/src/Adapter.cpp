#include <simpleble/Adapter.h>

#include "AdapterBase.h"
#include "AdapterBuilder.h"

#include <iostream>

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

Adapter::Adapter() {}

Adapter::~Adapter() {}

// TODO: Add validations to prevent calls into internal_ if not set.

std::string Adapter::identifier() { return internal_->identifier(); }

BluetoothAddress Adapter::address() { return internal_->address(); }

void Adapter::scan_start() { internal_->scan_start(); }

void Adapter::scan_stop() { internal_->scan_stop(); }

void Adapter::scan_for(int timeout_ms) { internal_->scan_for(timeout_ms); }

bool Adapter::scan_is_active() { return internal_->scan_is_active(); }

std::vector<Peripheral> Adapter::scan_get_results() { return internal_->scan_get_results(); }

void Adapter::set_callback_on_scan_start(std::function<void()> on_scan_start) {
    internal_->set_callback_on_scan_start(on_scan_start);
}

void Adapter::set_callback_on_scan_stop(std::function<void()> on_scan_stop) {
    internal_->set_callback_on_scan_stop(on_scan_stop);
}

void Adapter::set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated) {
    internal_->set_callback_on_scan_updated(on_scan_updated);
}

void Adapter::set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found) {
    internal_->set_callback_on_scan_found(on_scan_found);
}
