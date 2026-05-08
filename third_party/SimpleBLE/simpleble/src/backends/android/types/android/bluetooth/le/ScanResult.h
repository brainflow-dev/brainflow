#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

#include "types/android/bluetooth/BluetoothDevice.h"
#include "types/android/bluetooth/le/ScanRecord.h"

namespace SimpleBLE {
namespace Android {

class ScanResult {
  public:
    ScanResult(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    BluetoothDevice getDevice();
    int16_t getRssi();
    int16_t getTxPower();
    bool isConnectable();
    ScanRecord getScanRecord();
    std::string toString();

  private:
    // Underlying JNI object
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_getDevice;
    static jmethodID _method_getRssi;
    static jmethodID _method_getTxPower;
    static jmethodID _method_isConnectable;
    static jmethodID _method_getScanRecord;
    static jmethodID _method_toString;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<ScanResult> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE
