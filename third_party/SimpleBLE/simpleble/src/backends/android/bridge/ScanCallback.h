#pragma once

#include <kvn_safe_callback.hpp>
#include <map>
#include "types/android/bluetooth/le/ScanResult.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace SimpleBLE {
namespace Android {
namespace Bridge {

class ScanCallback {
  public:
    ScanCallback();
    virtual ~ScanCallback();
    jobject get() const { return _obj.get(); }  // TODO: Remove once nothing uses this

    void set_callback_onScanResult(std::function<void(Android::ScanResult)> callback);
    void set_callback_onBatchScanResults(std::function<void()> callback);
    void set_callback_onScanFailed(std::function<void()> callback);

    // Not for public use
    static void jni_onScanResultCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz, jint callback_type,
                                         Android::ScanResult scan_result);
    static void jni_onBatchScanResultsCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz,
                                              SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> results);
    static void jni_onScanFailedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz, jint error_code);

  private:
    // Underlying JNI object - Use SimpleJNI::Object with GlobalRef
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Map for instance tracking - Key should be comparable using JniObjectComparator
    static kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, ScanCallback*,
                         SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>>
        _map;

    // Callbacks
    kvn::safe_callback<void(Android::ScanResult)> _callback_onScanResult;
    kvn::safe_callback<void()> _callback_onBatchScanResults;
    kvn::safe_callback<void()> _callback_onScanFailed;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _constructor;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<ScanCallback> registrar;
};

}  // namespace Bridge
}  // namespace Android
}  // namespace SimpleBLE