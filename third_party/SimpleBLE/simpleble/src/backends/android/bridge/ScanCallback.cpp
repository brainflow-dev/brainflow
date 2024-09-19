#include <fmt/format.h>
#include <android/log.h>
#include <CommonUtils.h>
#include "ScanCallback.h"

namespace SimpleBLE {
namespace Android {
namespace Bridge {

JNI::Class ScanCallback::_cls;
std::map<jobject, ScanCallback*, JNI::JObjectComparator> ScanCallback::_map;

void ScanCallback::initialize() {
    JNI::Env env;

    if (_cls.get() == nullptr) {
        _cls = env.find_class("org/simpleble/android/bridge/ScanCallback");
    }
}

ScanCallback::ScanCallback() {
    initialize();

    _obj = _cls.call_constructor("()V");
    _map[_obj.get()] = this;
}

ScanCallback::~ScanCallback() {
    _map.erase(_obj.get());
}

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

void ScanCallback::jni_onScanResultCallback(JNIEnv *env, jobject thiz, jint callback_type, jobject j_scan_result) {
    auto it = ScanCallback::_map.find(thiz);
    if (it != ScanCallback::_map.end()) {
        ScanCallback* obj = it->second;
        Android::ScanResult scan_result(j_scan_result);

        auto msg = fmt::format("onScanResultCallback: {}", scan_result.toString());
        __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg.c_str());

        SAFE_CALLBACK_CALL(obj->_callback_onScanResult, scan_result);
    } else {
        // TODO: Throw an exception
    }
}

void ScanCallback::jni_onBatchScanResultsCallback(JNIEnv *env, jobject thiz, jobject results) {
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", "onBatchScanResultsCallback");
    // TODO: Implement
}

void ScanCallback::jni_onScanFailedCallback(JNIEnv *env, jobject thiz, jint error_code) {
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", "onScanFailedCallback");
    // TODO: Implement
}

}  // namespace Bridge
}  // namespace Android
}  // namespace SimpleBLE

extern "C" {
// clang-format off
JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_ScanCallback_onScanResultCallback(JNIEnv *env, jobject thiz, jint callback_type, jobject result) {
    SimpleBLE::Android::Bridge::ScanCallback::jni_onScanResultCallback(env, thiz, callback_type, result);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_ScanCallback_onScanFailedCallback(JNIEnv *env, jobject thiz, jint error_code) {
    SimpleBLE::Android::Bridge::ScanCallback::jni_onScanFailedCallback(env, thiz, error_code);
}
JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_ScanCallback_onBatchScanResultsCallback(JNIEnv *env, jobject thiz, jobject results) {
    SimpleBLE::Android::Bridge::ScanCallback::jni_onBatchScanResultsCallback(env, thiz, results);
}
// clang-format on
}