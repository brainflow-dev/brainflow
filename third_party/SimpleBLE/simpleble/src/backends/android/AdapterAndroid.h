#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Types.h>

#include "../common/AdapterBase.h"

#include <kvn_safe_callback.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "types/android/bluetooth/BluetoothAdapter.h"
#include "types/android/bluetooth/le/BluetoothScanner.h"
#include "bridge/ScanCallback.h"

namespace SimpleBLE {

class PeripheralAndroid;
class Peripheral;

class AdapterAndroid : public AdapterBase {
  public:
    AdapterAndroid();
    virtual ~AdapterAndroid();

    virtual void* underlying() const override;

    virtual std::string identifier() override;
    virtual BluetoothAddress address() override;

    virtual void power_on() override;
    virtual void power_off() override;
    virtual bool is_powered() override;

    virtual void scan_start() override;
    virtual void scan_stop() override;
    virtual void scan_for(int timeout_ms) override;
    virtual bool scan_is_active() override;
    virtual std::vector<std::shared_ptr<PeripheralBase>> scan_get_results() override;

    virtual std::vector<std::shared_ptr<PeripheralBase>> get_paired_peripherals() override;

    virtual bool bluetooth_enabled() override;

    // NOTE: The following methods have been made public to allow the JNI layer to call them, but
    // should not be called directly by the user.

    void onScanResultCallback(JNIEnv* env, jobject thiz, jint callback_type, jobject result);
    void onBatchScanResultsCallback(JNIEnv* env, jobject thiz, jobject results);
    void onScanFailedCallback(JNIEnv* env, jobject thiz, jint error_code);

  private:
    Android::BluetoothAdapter _btAdapter = Android::BluetoothAdapter::getDefaultAdapter();
    Android::BluetoothScanner _btScanner = _btAdapter.getBluetoothLeScanner();
    Android::Bridge::ScanCallback _btScanCallback;

    std::map<BluetoothAddress, std::shared_ptr<PeripheralAndroid>> peripherals_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralAndroid>> seen_peripherals_;

    std::atomic<bool> scanning_{false};
};

}  // namespace SimpleBLE
