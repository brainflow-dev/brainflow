#include "BluetoothGattCallback.h"
#include <CommonUtils.h>

#include <android/log.h>
#include <jni/Types.h>
#include <climits>

namespace SimpleBLE {
namespace Android {
namespace Bridge {

JNI::Class BluetoothGattCallback::_cls;
std::map<jobject, BluetoothGattCallback*, JNI::JObjectComparator> BluetoothGattCallback::_map;

void BluetoothGattCallback::initialize() {
    JNI::Env env;

    if (_cls.get() == nullptr) {
        _cls = env.find_class("org/simpleble/android/bridge/BluetoothGattCallback");
    }
}

BluetoothGattCallback::BluetoothGattCallback() : connected(false), mtu(UINT16_MAX) {
    initialize();

    _obj = _cls.call_constructor("()V");
    _map[_obj.get()] = this;
}

BluetoothGattCallback::~BluetoothGattCallback() { _map.erase(_obj.get()); }

void BluetoothGattCallback::set_callback_onConnectionStateChange(std::function<void(bool)> callback) {
    if (callback) {
        _callback_onConnectionStateChange.load(callback);
    } else {
        _callback_onConnectionStateChange.unload();
    }
}

void BluetoothGattCallback::set_callback_onServicesDiscovered(std::function<void(void)> callback) {
    if (callback) {
        _callback_onServicesDiscovered.load(callback);
    } else {
        _callback_onServicesDiscovered.unload();
    }
}

void BluetoothGattCallback::set_callback_onCharacteristicChanged(JNI::Object characteristic,
                                                                 std::function<void(std::vector<uint8_t>)> callback) {
    if (callback) {
        _callback_onCharacteristicChanged[characteristic].load(callback);
    } else {
        _callback_onCharacteristicChanged[characteristic].unload();
    }
}

void BluetoothGattCallback::clear_callback_onCharacteristicChanged(JNI::Object characteristic) {
    _callback_onCharacteristicChanged[characteristic].unload();
}

void BluetoothGattCallback::set_flag_characteristicWritePending(JNI::Object characteristic) {
    auto& flag_data = _flag_characteristicWritePending[characteristic];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_characteristicWritePending(JNI::Object characteristic) {
    auto& flag_data = _flag_characteristicWritePending[characteristic];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
    }
    flag_data.cv.notify_all();
}

void BluetoothGattCallback::wait_flag_characteristicWritePending(JNI::Object characteristic) {
    auto& flag_data = _flag_characteristicWritePending[characteristic];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // TODO CLEANUP
        throw std::runtime_error("Failed to write characteristic");
    }

    // TODO CLEANUP
}

void BluetoothGattCallback::set_flag_characteristicReadPending(JNI::Object characteristic) {
    auto& flag_data = _flag_characteristicReadPending[characteristic];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_characteristicReadPending(JNI::Object characteristic,
                                                                 std::vector<uint8_t> value) {
    auto& flag_data = _flag_characteristicReadPending[characteristic];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
        flag_data.value = value;
    }
    flag_data.cv.notify_all();
}

std::vector<uint8_t> BluetoothGattCallback::wait_flag_characteristicReadPending(JNI::Object characteristic) {
    auto& flag_data = _flag_characteristicReadPending[characteristic];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // TODO CLEANUP
        throw std::runtime_error("Failed to read characteristic");
    }

    return flag_data.value;
}

void BluetoothGattCallback::set_flag_descriptorWritePending(JNI::Object descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_descriptorWritePending(JNI::Object descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
    }
    flag_data.cv.notify_all();
}

void BluetoothGattCallback::wait_flag_descriptorWritePending(JNI::Object descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // TODO CLEANUP
        throw std::runtime_error("Failed to write descriptor");
    }

    // TODO CLEANUP
}

void BluetoothGattCallback::set_flag_descriptorReadPending(JNI::Object descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_descriptorReadPending(JNI::Object descriptor, std::vector<uint8_t> value) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
        flag_data.value = value;
    }
    flag_data.cv.notify_all();
}

std::vector<uint8_t> BluetoothGattCallback::wait_flag_descriptorReadPending(JNI::Object descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // TODO CLEANUP
        throw std::runtime_error("Failed to read descriptor");
    }

    return flag_data.value;
}

void BluetoothGattCallback::jni_onConnectionStateChangeCallback(JNIEnv* env, jobject thiz, jobject gatt, jint status,
                                                                jint new_state) {
    auto msg = fmt::format("onConnectionStateChangeCallback status: {} new_state: {}", status, new_state);
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg.c_str());

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }

    BluetoothGattCallback* obj = it->second;
    const bool connected = new_state == 2;
    obj->connected = connected;
    SAFE_CALLBACK_CALL(obj->_callback_onConnectionStateChange, connected);
}

void BluetoothGattCallback::jni_onServicesDiscoveredCallback(JNIEnv* env, jobject thiz, jobject gatt, jint status) {
    auto msg = "onServicesDiscoveredCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }

    BluetoothGattCallback* obj = it->second;
    obj->services_discovered = true;
    SAFE_CALLBACK_CALL(obj->_callback_onServicesDiscovered);
}

void BluetoothGattCallback::jni_onServiceChangedCallback(JNIEnv* env, jobject thiz, jobject gatt) {
    // NOTE: If this one gets triggered we're kinda screwed.
    auto msg = "onServiceChangedCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }
}

void BluetoothGattCallback::jni_onCharacteristicChangedCallback(JNIEnv* env, jobject thiz, jobject gatt,
                                                                jobject characteristic, jbyteArray value) {
    auto msg = "onCharacteristicChangedCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }

    BluetoothGattCallback* obj = it->second;
    auto& callback = obj->_callback_onCharacteristicChanged[characteristic];
    if (callback) {
        std::vector<uint8_t> data = JNI::Types::fromJByteArray(value);
        SAFE_CALLBACK_CALL(callback, data);
    }
}

void BluetoothGattCallback::jni_onCharacteristicReadCallback(JNIEnv* env, jobject thiz, jobject gatt,
                                                             jobject characteristic, jbyteArray value, jint status) {
    auto msg = "onCharacteristicReadCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }

    BluetoothGattCallback* obj = it->second;
    obj->clear_flag_characteristicReadPending(characteristic, JNI::Types::fromJByteArray(value));
}

void BluetoothGattCallback::jni_onCharacteristicWriteCallback(JNIEnv* env, jobject thiz, jobject gatt,
                                                              jobject characteristic, jint status) {
    auto msg = "onCharacteristicWriteCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }

    BluetoothGattCallback* obj = it->second;
    obj->clear_flag_characteristicWritePending(characteristic);
}

void BluetoothGattCallback::jni_onDescriptorReadCallback(JNIEnv* env, jobject thiz, jobject gatt, jobject descriptor,
                                                         jbyteArray value, jint status) {
    auto msg = "onDescriptorReadCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }

    BluetoothGattCallback* obj = it->second;
    obj->clear_flag_descriptorReadPending(descriptor, JNI::Types::fromJByteArray(value));
}

void BluetoothGattCallback::jni_onDescriptorWriteCallback(JNIEnv* env, jobject thiz, jobject gatt, jobject descriptor,
                                                          jint status) {
    auto msg = "onDescriptorWriteCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it == BluetoothGattCallback::_map.end()) {
        // TODO: Throw an exception
        return;
    }

    BluetoothGattCallback* obj = it->second;
    obj->clear_flag_descriptorWritePending(descriptor);
}

void BluetoothGattCallback::jni_onMtuChangedCallback(JNIEnv* env, jobject thiz, jobject gatt, jint mtu, jint status) {
    auto msg = "onMtuChangedCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);

    auto it = BluetoothGattCallback::_map.find(thiz);
    if (it != BluetoothGattCallback::_map.end()) {
        BluetoothGattCallback* obj = it->second;
        obj->mtu = mtu;
    } else {
        // TODO: Throw an exception
    }
}

void BluetoothGattCallback::jni_onPhyReadCallback(JNIEnv* env, jobject thiz, jobject gatt, jint tx_phy, jint rx_phy,
                                                  jint status) {
    auto msg = "onPhyReadCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);
}

void BluetoothGattCallback::jni_onPhyUpdateCallback(JNIEnv* env, jobject thiz, jobject gatt, jint tx_phy, jint rx_phy,
                                                    jint status) {
    auto msg = "onPhyUpdateCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);
}

void BluetoothGattCallback::jni_onReadRemoteRssiCallback(JNIEnv* env, jobject thiz, jobject gatt, jint rssi,
                                                         jint status) {
    auto msg = "onReadRemoteRssiCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);
}

void BluetoothGattCallback::jni_onReliableWriteCompletedCallback(JNIEnv* env, jobject thiz, jobject gatt, jint status) {
    auto msg = "onReliableWriteCompletedCallback";
    __android_log_write(ANDROID_LOG_INFO, "SimpleBLE", msg);
}

}  // namespace Bridge
}  // namespace Android
}  // namespace SimpleBLE

extern "C" {
// clang-format off
JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onCharacteristicChangedCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject characteristic, jbyteArray value) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onCharacteristicChangedCallback(env, thiz, gatt, characteristic, value);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onCharacteristicReadCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject characteristic, jbyteArray value, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onCharacteristicReadCallback(env, thiz, gatt, characteristic, value, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onCharacteristicWriteCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject characteristic, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onCharacteristicWriteCallback(env, thiz, gatt, characteristic, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onConnectionStateChangeCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint status, jint new_state) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onConnectionStateChangeCallback(env, thiz, gatt, status, new_state);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onDescriptorReadCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject descriptor, jbyteArray value, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onDescriptorReadCallback(env, thiz, gatt, descriptor, value, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onDescriptorWriteCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject descriptor, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onDescriptorWriteCallback(env, thiz, gatt, descriptor, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onMtuChangedCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint mtu, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onMtuChangedCallback(env, thiz, gatt, mtu, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onPhyReadCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint tx_phy, jint rx_phy, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onPhyReadCallback(env, thiz, gatt, tx_phy, rx_phy, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onPhyUpdateCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint tx_phy, jint rx_phy, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onPhyUpdateCallback(env, thiz, gatt, tx_phy, rx_phy, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onReadRemoteRssiCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint rssi, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onReadRemoteRssiCallback(env, thiz, gatt, rssi, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onReliableWriteCompletedCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onReliableWriteCompletedCallback(env, thiz, gatt, status);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onServiceChangedCallback(
    JNIEnv* env, jobject thiz, jobject gatt) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onServiceChangedCallback(env, thiz, gatt);
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onServicesDiscoveredCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint status) {
    SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onServicesDiscoveredCallback(env, thiz, gatt, status);
}
// clang-format on
}  // extern "C"