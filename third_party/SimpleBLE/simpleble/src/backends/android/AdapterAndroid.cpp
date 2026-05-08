#include "AdapterAndroid.h"
#include "BackendAndroid.h"
#include "BuilderBase.h"
#include "CommonUtils.h"
#include "PeripheralAndroid.h"
#include "simpleble/Peripheral.h"

#include <types/android/bluetooth/BluetoothDevice.h>
#include <types/android/bluetooth/le/ScanResult.h>
#include <android/log.h>
#include <fmt/core.h>
#include <jni.h>
#include <thread>

using namespace SimpleBLE;

bool AdapterAndroid::bluetooth_enabled() { return BackendAndroid::get()->bluetooth_enabled(); }

AdapterAndroid::AdapterAndroid() {
    _btScanCallback.set_callback_onScanResult([this](Android::ScanResult scan_result) {
        std::string address = scan_result.getDevice().getAddress();

        if (this->peripherals_.count(address) == 0) {
            // If the incoming peripheral has never been seen before, create and save a reference to it.
            auto base_peripheral = std::make_shared<PeripheralAndroid>(scan_result.getDevice());
            this->peripherals_.insert(std::make_pair(address, base_peripheral));
        }

        // Update the received advertising data.
        auto base_peripheral = this->peripherals_.at(address);
        base_peripheral->update_advertising_data(scan_result);

        // Convert the base object into an external-facing Peripheral object
        Peripheral peripheral = Factory::build(base_peripheral);

        // Check if the device has been seen before, to forward the correct call to the user.
        if (this->seen_peripherals_.count(address) == 0) {
            // Store it in our table of seen peripherals
            this->seen_peripherals_.insert(std::make_pair(address, base_peripheral));
            SAFE_CALLBACK_CALL(this->_callback_on_scan_found, peripheral);
        } else {
            SAFE_CALLBACK_CALL(this->_callback_on_scan_updated, peripheral);
        }
    });
}

AdapterAndroid::~AdapterAndroid() {}

void* AdapterAndroid::underlying() const { return nullptr; }

std::string AdapterAndroid::identifier() { return _btAdapter.getName(); }

BluetoothAddress AdapterAndroid::address() { return BluetoothAddress(_btAdapter.getAddress()); }

void AdapterAndroid::power_on() {
    // NOTE: This feature was deprecated in API level 33.
    // Despite us targeting API level 31, we'll play nicely and not call this method.
    // https://developer.android.com/reference/android/bluetooth/BluetoothAdapter#enable()
}

void AdapterAndroid::power_off() {
    // NOTE: This feature was deprecated in API level 33.
    // Despite us targeting API level 31, we'll play nicely and not call this method.
    // https://developer.android.com/reference/android/bluetooth/BluetoothAdapter#disable()
}

bool AdapterAndroid::is_powered() { return _btAdapter.isEnabled(); }

void AdapterAndroid::scan_start() {
    seen_peripherals_.clear();
    _btScanner.startScan(_btScanCallback);
    scanning_ = true;
    SAFE_CALLBACK_CALL(this->_callback_on_scan_start);
}

void AdapterAndroid::scan_stop() {
    _btScanner.stopScan(_btScanCallback);
    scanning_ = false;
    SAFE_CALLBACK_CALL(this->_callback_on_scan_stop);
}

void AdapterAndroid::scan_for(int timeout_ms) {
    scan_start();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
    scan_stop();
}

bool AdapterAndroid::scan_is_active() { return scanning_; }

SharedPtrVector<PeripheralBase> AdapterAndroid::scan_get_results() { return Util::values(seen_peripherals_); }

SharedPtrVector<PeripheralBase> AdapterAndroid::get_paired_peripherals() {
    SharedPtrVector<PeripheralBase> peripherals;

    auto paired_list = _btAdapter.getBondedDevices();
    for (auto& device : paired_list) {
        peripherals.push_back(std::make_shared<PeripheralAndroid>(device));
    }

    return peripherals;
}
