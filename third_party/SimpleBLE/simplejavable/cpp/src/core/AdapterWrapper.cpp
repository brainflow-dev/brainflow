#include "AdapterWrapper.h"
#include "Cache.h"
#include "PeripheralWrapper.h"

AdapterWrapper::AdapterWrapper(const SimpleBLE::Adapter& adapter) : _adapter(adapter) {}

AdapterWrapper::~AdapterWrapper() {
    _adapter.set_callback_on_scan_found(nullptr);
    _adapter.set_callback_on_scan_updated(nullptr);
    _adapter.set_callback_on_scan_start(nullptr);
    _adapter.set_callback_on_scan_stop(nullptr);
}

SimpleBLE::Adapter& AdapterWrapper::get() { return _adapter; }

const SimpleBLE::Adapter& AdapterWrapper::get() const { return _adapter; }

void AdapterWrapper::setCallback(Org::SimpleJavaBLE::AdapterCallback& callback) {
    _callback = std::move(callback);

    _adapter.set_callback_on_scan_found([this](const SimpleBLE::Peripheral& peripheral) {
        PeripheralWrapper peripheral_wrapper(peripheral);
        Cache::get().addPeripheral(getHash(), peripheral_wrapper.getHash(), peripheral_wrapper);
        _callback.on_scan_found(peripheral_wrapper.getHash());
    });

    _adapter.set_callback_on_scan_updated([this](const SimpleBLE::Peripheral& peripheral) {
        PeripheralWrapper peripheral_wrapper(peripheral);
        _callback.on_scan_updated(peripheral_wrapper.getHash());
    });

    _adapter.set_callback_on_scan_start([this]() { _callback.on_scan_start(); });

    _adapter.set_callback_on_scan_stop([this]() { _callback.on_scan_stop(); });
}

size_t AdapterWrapper::getHash() { return std::hash<std::string>{}(_adapter.identifier()); }
