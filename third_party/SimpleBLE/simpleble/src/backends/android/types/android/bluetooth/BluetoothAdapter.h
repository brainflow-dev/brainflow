#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"
#include "types/android/bluetooth/BluetoothDevice.h"
#include "types/android/bluetooth/le/BluetoothScanner.h"

namespace SimpleBLE {
namespace Android {

class BluetoothAdapter {
  public:
    BluetoothAdapter(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);
    virtual ~BluetoothAdapter() = default;

    std::string getName();
    std::string getAddress();
    bool isEnabled();
    int getState();

    BluetoothScanner getBluetoothLeScanner();
    std::vector<BluetoothDevice> getBondedDevices();

    // NOTE: The correct way to request a BluetoothAdapter is to go though the BluetoothManager,
    // as described in https://developer.android.com/reference/android/bluetooth/BluetoothManager#getAdapter()
    // However, for simplicity, we are using a direct call to BluetoothAdapter.getDefaultAdapter() which is
    // deprecated in API 31 but still works. We'll need to implement a backend bypass to get a Context
    // object and call getSystemService(Context.BLUETOOTH_SERVICE) to get the BluetoothManager.

    // NOTE: Android BluetoothAdapter and BluetoothScanner classes are singletons, but so is this
    // class, so we don't need to make them static.
    static BluetoothAdapter getDefaultAdapter();
    jobject get() const { return _obj.get(); }  // TODO: Remove once nothing uses this

  private:
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_getName;
    static jmethodID _method_getAddress;
    static jmethodID _method_isEnabled;
    static jmethodID _method_getState;
    static jmethodID _method_getBluetoothLeScanner;
    static jmethodID _method_getBondedDevices;
    static jmethodID _method_getDefaultAdapter;

    // JNI descriptors for auto-registration
    static const SimpleJNI::JNIDescriptor instance_descriptor;
    static const SimpleJNI::StaticJNIDescriptor static_descriptor;
    static const SimpleJNI::AutoRegister<BluetoothAdapter> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE