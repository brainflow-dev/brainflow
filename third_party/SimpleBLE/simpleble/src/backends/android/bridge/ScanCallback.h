#pragma once

#include "jni/Common.hpp"
#include <kvn_safe_callback.hpp>
#include "android/ScanResult.h"
#include <map>

namespace SimpleBLE {
namespace Android {
namespace Bridge {

class ScanCallback {
  public:
    ScanCallback();
    virtual ~ScanCallback();
    jobject get() { return _obj.get(); } // TODO: Remove once nothing uses this

    void set_callback_onScanResult(std::function<void(Android::ScanResult)> callback);
    void set_callback_onBatchScanResults(std::function<void()> callback);
    void set_callback_onScanFailed(std::function<void()> callback);

    // Not for public use
    static void jni_onScanResultCallback(JNIEnv *env, jobject thiz, jint callback_type, jobject result);
    static void jni_onBatchScanResultsCallback(JNIEnv *env, jobject thiz, jobject results);
    static void jni_onScanFailedCallback(JNIEnv *env, jobject thiz, jint error_code);

  private:
    static JNI::Class _cls;
    static std::map<jobject, ScanCallback*, JNI::JObjectComparator> _map;
    static void initialize();

    JNI::Object _obj;

    kvn::safe_callback<void(Android::ScanResult)> _callback_onScanResult;
    kvn::safe_callback<void()> _callback_onBatchScanResults;
    kvn::safe_callback<void()> _callback_onScanFailed;
};

}  // namespace Bridge
}  // namespace Android
}  // namespace SimpleBLE