#include <simpleble/AdapterSafe.h>

using namespace SimpleBLE;

using SPeripheral = SimpleBLE::Safe::Peripheral;
using SAdapter = SimpleBLE::Safe::Adapter;

using UAdapter = SimpleBLE::Adapter;

SAdapter::Adapter(UAdapter& adapter) : internal_(adapter) {}
SAdapter::Adapter(UAdapter&& adapter) : internal_(std::move(adapter)) {}

std::optional<std::string> SAdapter::identifier() noexcept {
    try {
        return internal_.identifier();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<SimpleBLE::BluetoothAddress> SAdapter::address() noexcept {
    try {
        return internal_.address();
    } catch (...) {
        return std::nullopt;
    }
}

SAdapter::operator SimpleBLE::Adapter() const noexcept { return internal_; }

bool SAdapter::scan_start() noexcept {
    try {
        internal_.scan_start();
        return true;
    } catch (...) {
        return false;
    }
}

bool SAdapter::scan_stop() noexcept {
    try {
        internal_.scan_stop();
        return true;
    } catch (...) {
        return false;
    }
}

bool SAdapter::scan_for(int timeout_ms) noexcept {
    try {
        internal_.scan_for(timeout_ms);
        return true;
    } catch (...) {
        return false;
    }
}

std::optional<bool> SAdapter::scan_is_active() noexcept {
    try {
        return internal_.scan_is_active();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::vector<SPeripheral>> SAdapter::scan_get_results() noexcept {
    try {
        std::vector<SPeripheral> r;
        for (auto p : internal_.scan_get_results()) {
            r.push_back(std::move(p));
        }
        return r;
    } catch (...) {
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<std::vector<SPeripheral>> SAdapter::get_paired_peripherals() noexcept {
    try {
        std::vector<SPeripheral> r;
        for (auto p : internal_.get_paired_peripherals()) {
            r.push_back(std::move(p));
        }
        return r;
    } catch (...) {
        return std::nullopt;
    }
    return std::nullopt;
}

bool SAdapter::set_callback_on_scan_start(std::function<void()> on_scan_start) noexcept {
    try {
        internal_.set_callback_on_scan_start(std::move(on_scan_start));
        return true;
    } catch (...) {
        return false;
    }
}

bool SAdapter::set_callback_on_scan_stop(std::function<void()> on_scan_stop) noexcept {
    try {
        internal_.set_callback_on_scan_stop(std::move(on_scan_stop));
        return true;
    } catch (...) {
        return false;
    }
}

bool SAdapter::set_callback_on_scan_updated(std::function<void(SPeripheral)> on_scan_updated) noexcept {
    try {
        internal_.set_callback_on_scan_updated(
            [on_scan_updated = std::move(on_scan_updated)](auto p) { on_scan_updated(p); });
        return true;
    } catch (...) {
        return false;
    }
}

bool SAdapter::set_callback_on_scan_found(std::function<void(SPeripheral)> on_scan_found) noexcept {
    try {
        internal_.set_callback_on_scan_found([on_scan_found = std::move(on_scan_found)](auto p) { on_scan_found(p); });
        return true;
    } catch (...) {
        return false;
    }
}

// NOTE: this should be the implementation once per-adapters are supported
/*
std::optional<bool> SAdapter::bluetooth_enabled() noexcept {
    try {
        return internal_.bluetooth_enabled();
    } catch (...) {
        return std::nullopt;
    }
}
*/
std::optional<bool> SAdapter::bluetooth_enabled() noexcept {
    try {
        return UAdapter::bluetooth_enabled();
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::vector<SAdapter>> SAdapter::get_adapters() noexcept {
    try {
        auto adapters = UAdapter::get_adapters();
        std::vector<SAdapter> safe_adapters;
        for (auto& adapter : adapters) {
            safe_adapters.push_back(std::move(adapter));
        }
        return safe_adapters;
    } catch (...) {
        return std::nullopt;
    }
}
