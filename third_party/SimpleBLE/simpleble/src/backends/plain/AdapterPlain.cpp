#include <simpleble/Peripheral.h>

#include "AdapterPlain.h"
#include "BuilderBase.h"
#include "CommonUtils.h"
#include "PeripheralBase.h"
#include "PeripheralPlain.h"

#include <memory>
#include <thread>

using namespace SimpleBLE;

bool AdapterPlain::bluetooth_enabled() { return true; }

AdapterPlain::AdapterPlain() {}

AdapterPlain::~AdapterPlain() {}

void* AdapterPlain::underlying() const { return nullptr; }

std::string AdapterPlain::identifier() { return "Plain Adapter"; }

BluetoothAddress AdapterPlain::address() { return "AA:BB:CC:DD:EE:FF"; }

void AdapterPlain::power_on() {}

void AdapterPlain::power_off() {}

bool AdapterPlain::is_powered() { return true; }

void AdapterPlain::scan_start() {
    is_scanning_ = true;
    SAFE_CALLBACK_CALL(this->_callback_on_scan_start);

    Peripheral peripheral = Factory::build(std::make_shared<PeripheralPlain>());
    SAFE_CALLBACK_CALL(this->_callback_on_scan_found, peripheral);
    SAFE_CALLBACK_CALL(this->_callback_on_scan_updated, peripheral);
}

void AdapterPlain::scan_stop() {
    is_scanning_ = false;
    SAFE_CALLBACK_CALL(this->_callback_on_scan_stop);
}

void AdapterPlain::scan_for(int timeout_ms) {
    scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    scan_stop();
}

bool AdapterPlain::scan_is_active() { return is_scanning_; }
SharedPtrVector<PeripheralBase> AdapterPlain::scan_get_results() {
    SharedPtrVector<PeripheralBase> peripherals;
    peripherals.push_back(std::make_shared<PeripheralPlain>());

    return peripherals;
}

SharedPtrVector<PeripheralBase> AdapterPlain::get_paired_peripherals() {
    SharedPtrVector<PeripheralBase> peripherals;
    peripherals.push_back(std::make_shared<PeripheralPlain>());

    return peripherals;
}
