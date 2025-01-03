#pragma once

#include <simpleble/Exceptions.h>
#include <simpleble/Peripheral.h>
#include <simpleble/Types.h>

#include <kvn_safe_callback.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "jni/Common.hpp"
#include "bridge/ScanCallback.h"

namespace SimpleBLE {

class AdapterBase {
  public:
    AdapterBase();
    virtual ~AdapterBase();

    void* underlying() const;

    std::string identifier();
    BluetoothAddress address();

    void scan_start();
    void scan_stop();
    void scan_for(int timeout_ms);
    bool scan_is_active();
    std::vector<Peripheral> scan_get_results();

    void set_callback_on_scan_start(std::function<void()> on_scan_start);
    void set_callback_on_scan_stop(std::function<void()> on_scan_stop);
    void set_callback_on_scan_updated(std::function<void(Peripheral)> on_scan_updated);
    void set_callback_on_scan_found(std::function<void(Peripheral)> on_scan_found);

    std::vector<Peripheral> get_paired_peripherals();

    static bool bluetooth_enabled();
    static std::vector<std::shared_ptr<AdapterBase>> get_adapters();

    // NOTE: The following methods have been made public to allow the JNI layer to call them, but
    // should not be called directly by the user.

    void onScanResultCallback(JNIEnv *env, jobject thiz, jint callback_type, jobject result);
    void onBatchScanResultsCallback(JNIEnv *env, jobject thiz, jobject results);
    void onScanFailedCallback(JNIEnv *env, jobject thiz, jint error_code);

    //static std::map<jobject, AdapterBase*, JNI::JObjectComparator> _scanCallbackMap;

  private:
    // NOTE: The correct way to request a BluetoothAdapter is to go though the BluetoothManager,
    // as described in https://developer.android.com/reference/android/bluetooth/BluetoothManager#getAdapter()
    // However, for simplicity, we are using a direct call to BluetoothAdapter.getDefaultAdapter() which is
    // deprecated in API 31 but still works. We'll need to implement a backend bypass to get a Context
    // object and call getSystemService(Context.BLUETOOTH_SERVICE) to get the BluetoothManager.

    void static initialize();
    // NOTE: Android BluetoothAdapter and BluetoothScanner classes are singletons, so we can use a static instance.
    static JNI::Class _btAdapterCls;
    static JNI::Class _btScanCallbackCls;
    static JNI::Class _btScanResultCls;
    static JNI::Object _btAdapter;
    static JNI::Object _btScanner;

    Android::Bridge::ScanCallback _btScanCallback;

    std::map<BluetoothAddress, std::shared_ptr<PeripheralBase>> peripherals_;
    std::map<BluetoothAddress, std::shared_ptr<PeripheralBase>> seen_peripherals_;

    kvn::safe_callback<void()> callback_on_scan_start_;
    kvn::safe_callback<void()> callback_on_scan_stop_;
    kvn::safe_callback<void(Peripheral)> callback_on_scan_updated_;
    kvn::safe_callback<void(Peripheral)> callback_on_scan_found_;

    std::atomic<bool> scanning_{false};


};

}  // namespace SimpleBLE
