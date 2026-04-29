#pragma once

#include "simplejni/Common.hpp"
#include "bridge/ScanCallback.h"

namespace SimpleBLE {
namespace Android {

class BluetoothScanner {
  public:
    BluetoothScanner(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    void startScan(Bridge::ScanCallback& callback);
    void stopScan(Bridge::ScanCallback& callback);

    std::string toString();

    jobject get() const { return _obj.get(); }  // TODO: Remove once nothing uses this

  private:
    // Underlying JNI object - Use SimpleJNI::Object with GlobalRef
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _constructor;
    static jmethodID _method_startScan;
    static jmethodID _method_stopScan;
    static jmethodID _method_toString;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<BluetoothScanner> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE
