#include "ScanCallback.h"
#include <CommonUtils.h>
#include <android/log.h>
#include <fmt/format.h>

namespace SimpleBLE {
namespace Android {
namespace Bridge {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> ScanCallback::_cls;
jmethodID ScanCallback::_constructor = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor ScanCallback::descriptor{
    "org/simpleble/android/bridge/ScanCallback", // Java class name
    &_cls,                                       // Where to store the jclass
    {                                            // Methods to preload
     {"<init>", "()V", &_constructor}            // Constructor
    }};

const SimpleJNI::AutoRegister<ScanCallback> ScanCallback::registrar{&descriptor};

kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, ScanCallback*,
                     SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>>
    ScanCallback::_map;

ScanCallback::ScanCallback() : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("ScanCallback JNI resources not preloaded. Ensure SimpleJNI::Registrar::preload() is called.");
    }

    SimpleJNI::Env env;
    jobject local_obj = env->NewObject(_cls.get(), _constructor);
    if (local_obj == nullptr) {
        throw std::runtime_error("Failed to create ScanCallback Java instance");
    }

    _obj = SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>(local_obj);
    env->DeleteLocalRef(local_obj);

    ScanCallback::_map.insert(_obj, this);
}

ScanCallback::~ScanCallback() { ScanCallback::_map.erase(_obj); }

void ScanCallback::set_callback_onScanResult(std::function<void(Android::ScanResult)> callback) {
    if (callback) {
        _callback_onScanResult.load(callback);
    } else {
        _callback_onScanResult.unload();
    }
}

void ScanCallback::set_callback_onBatchScanResults(std::function<void()> callback) {
    if (callback) {
        _callback_onBatchScanResults.load(callback);
    } else {
        _callback_onBatchScanResults.unload();
    }
}

void ScanCallback::set_callback_onScanFailed(std::function<void()> callback) {
    if (callback) {
        _callback_onScanFailed.load(callback);
    } else {
        _callback_onScanFailed.unload();
    }
}

void ScanCallback::jni_onScanResultCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz, jint callback_type,
                                            SimpleBLE::Android::ScanResult scan_result) {
    auto callback_opt = ScanCallback::_map.get(thiz);
    if (!callback_opt) {
        SIMPLEBLE_LOG_FATAL("Failed to find ScanCallback object. This should never happen.");
        // TODO: Throw an exception
        return;
    }

    ScanCallback* obj = callback_opt.value();

    auto msg = fmt::format("onScanResultCallback: {}", scan_result.toString());
    __android_log_write(ANDROID_LOG_DEBUG, "SimpleBLE", msg.c_str());

    SAFE_CALLBACK_CALL(obj->_callback_onScanResult, scan_result);
}

void ScanCallback::jni_onBatchScanResultsCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz,
                                                 SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> results) {
    auto callback_opt = ScanCallback::_map.get(thiz);
    if (!callback_opt) {
        SIMPLEBLE_LOG_FATAL("Failed to find ScanCallback object. This should never happen.");
        // TODO: Throw an exception
        return;
    }

    __android_log_write(ANDROID_LOG_DEBUG, "SimpleBLE", "onBatchScanResultsCallback");
    // TODO: Implement
}

void ScanCallback::jni_onScanFailedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz, jint error_code) {
    auto callback_opt = ScanCallback::_map.get(thiz);
    if (!callback_opt) {
        SIMPLEBLE_LOG_FATAL("Failed to find ScanCallback object. This should never happen.");
        // TODO: Throw an exception
        return;
    }

    __android_log_write(ANDROID_LOG_DEBUG, "SimpleBLE", "onScanFailedCallback");
    // TODO: Implement
}
}  // namespace Bridge
}  // namespace Android
}  // namespace SimpleBLE

extern "C" {
// clang-format off
JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_ScanCallback_onScanResultCallback(JNIEnv *env, jobject thiz, jint callback_type, jobject result) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> result_obj(result);
    SimpleBLE::Android::ScanResult scan_result(result_obj);
    SimpleJNI::Runner::get().enqueue([thiz_obj, callback_type, scan_result]() {
        SimpleBLE::Android::Bridge::ScanCallback::jni_onScanResultCallback(thiz_obj, callback_type, scan_result);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_ScanCallback_onScanFailedCallback(JNIEnv *env, jobject thiz, jint error_code) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, error_code]() {
        SimpleBLE::Android::Bridge::ScanCallback::jni_onScanFailedCallback(thiz_obj, error_code);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_ScanCallback_onBatchScanResultsCallback(JNIEnv *env, jobject thiz, jobject results) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> results_obj(results);
    SimpleJNI::Runner::get().enqueue([thiz_obj, results_obj]() {
        SimpleBLE::Android::Bridge::ScanCallback::jni_onBatchScanResultsCallback(thiz_obj, results_obj);
    });
}
// clang-format on
}