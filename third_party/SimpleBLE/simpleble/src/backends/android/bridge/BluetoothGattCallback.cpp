#include "BluetoothGattCallback.h"
#include <CommonUtils.h>

#include <simplejni/Common.hpp>
#include <climits>
#include "LoggingInternal.h"

namespace SimpleBLE {
namespace Android {
namespace Bridge {

SimpleJNI::GlobalRef<jclass> BluetoothGattCallback::_cls;
jmethodID BluetoothGattCallback::_constructor = nullptr;
kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, BluetoothGattCallback*, SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>> BluetoothGattCallback::_map;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor BluetoothGattCallback::descriptor{
    "org/simpleble/android/bridge/BluetoothGattCallback", // Java class name
    &_cls,                                                // Where to store the jclass
    {                                                     // Methods to preload
     {"<init>", "()V", &_constructor}                     // Constructor
    }};

const SimpleJNI::AutoRegister<BluetoothGattCallback> BluetoothGattCallback::registrar{&descriptor};

#define GET_CALLBACK_OBJECT_OR_RETURN(thiz)                                                                \
    ({                                                                                                     \
        auto callback_opt = BluetoothGattCallback::_map.get(thiz);                                         \
        if (!callback_opt) {                                                                               \
            SIMPLEBLE_LOG_FATAL("Failed to find BluetoothGattCallback object. This should never happen."); \
            return;                                                                                        \
        }                                                                                                  \
        callback_opt.value();                                                                              \
    })

BluetoothGattCallback::BluetoothGattCallback() : connected(false), services_discovered(false), mtu(UINT16_MAX) {
    if (!_cls.get()) {
        throw std::runtime_error("BluetoothGattCallback JNI resources not preloaded. Ensure SimpleJNI::Registrar::preload() is called.");
    }

    SimpleJNI::Env env;
    jobject local_obj = env->NewObject(_cls.get(), _constructor);
    if (local_obj == nullptr) {
        throw std::runtime_error("Failed to create BluetoothGattCallback Java instance");
    }

    _obj = SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>(local_obj);
    env->DeleteLocalRef(local_obj);

    _map.insert(_obj, this);
}

BluetoothGattCallback::~BluetoothGattCallback() { _map.erase(_obj); }

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

void BluetoothGattCallback::set_callback_onCharacteristicChanged(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic,
                                                                 std::function<void(std::vector<uint8_t>)> callback) {
    if (callback) {
        _callback_onCharacteristicChanged[characteristic].load(callback);
    } else {
        _callback_onCharacteristicChanged[characteristic].unload();
    }
}

void BluetoothGattCallback::clear_callback_onCharacteristicChanged(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic) {
    _callback_onCharacteristicChanged[characteristic].unload();
}

void BluetoothGattCallback::set_flag_characteristicWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic) {
    auto& flag_data = _flag_characteristicWritePending[characteristic];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_characteristicWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic) {
    auto& flag_data = _flag_characteristicWritePending[characteristic];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
    }
    flag_data.cv.notify_all();
}

void BluetoothGattCallback::wait_flag_characteristicWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic) {
    auto& flag_data = _flag_characteristicWritePending[characteristic];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // Timeout has occurred.
        throw std::runtime_error("Failed to write characteristic");
    }
}

void BluetoothGattCallback::set_flag_characteristicReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic) {
    auto& flag_data = _flag_characteristicReadPending[characteristic];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_characteristicReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic,
                                                                 std::vector<uint8_t> value) {
    auto& flag_data = _flag_characteristicReadPending[characteristic];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
        flag_data.value = value;
    }
    flag_data.cv.notify_all();
}

std::vector<uint8_t> BluetoothGattCallback::wait_flag_characteristicReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic) {
    auto& flag_data = _flag_characteristicReadPending[characteristic];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // Timeout has occurred.
        throw std::runtime_error("Failed to read characteristic");
    }

    return flag_data.value;
}

void BluetoothGattCallback::set_flag_descriptorWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_descriptorWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
    }
    flag_data.cv.notify_all();
}

void BluetoothGattCallback::wait_flag_descriptorWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor) {
    auto& flag_data = _flag_descriptorWritePending[descriptor];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // Timeout has occurred.
        throw std::runtime_error("Failed to write descriptor");
    }
}

void BluetoothGattCallback::set_flag_descriptorReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor) {
    auto& flag_data = _flag_descriptorReadPending[descriptor];

    std::lock_guard<std::mutex> lock(flag_data.mtx);
    flag_data.flag = true;
}

void BluetoothGattCallback::clear_flag_descriptorReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor, std::vector<uint8_t> value) {
    auto& flag_data = _flag_descriptorReadPending[descriptor];
    {
        std::lock_guard<std::mutex> lock(flag_data.mtx);
        flag_data.flag = false;
        flag_data.value = value;
    }
    flag_data.cv.notify_all();
}

std::vector<uint8_t> BluetoothGattCallback::wait_flag_descriptorReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor) {
    auto& flag_data = _flag_descriptorReadPending[descriptor];
    std::unique_lock<std::mutex> lock(flag_data.mtx);
    flag_data.cv.wait_for(lock, std::chrono::seconds(5), [&flag_data] { return !flag_data.flag; });

    if (flag_data.flag) {
        // Timeout has occurred.
        throw std::runtime_error("Failed to read descriptor");
    }

    return flag_data.value;
}

// JNI Callbacks

void BluetoothGattCallback::jni_onConnectionStateChangeCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint status, jint new_state) {
    auto msg = fmt::format("onConnectionStateChangeCallback status: {} new_state: {}", status, new_state);
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    const bool connected = new_state == 2;
    obj->connected = connected;
    SAFE_CALLBACK_CALL(obj->_callback_onConnectionStateChange, connected);
}

void BluetoothGattCallback::jni_onServicesDiscoveredCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint status) {
    auto msg = "onServicesDiscoveredCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    obj->services_discovered = true;
    SAFE_CALLBACK_CALL(obj->_callback_onServicesDiscovered);
}

void BluetoothGattCallback::jni_onServiceChangedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj) {
    // NOTE: If this one gets triggered we're kinda screwed.
    auto msg = "onServiceChangedCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
}

void BluetoothGattCallback::jni_onCharacteristicChangedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj,
                                                                SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value) {
    auto msg = "onCharacteristicChangedCallback with value: " + value.bytes().toHex(true);
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    auto& callback = obj->_callback_onCharacteristicChanged[characteristic_obj];
    if (callback) {
        SAFE_CALLBACK_CALL(callback, value.bytes());
    }
}

void BluetoothGattCallback::jni_onCharacteristicReadCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj,
                                                             SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value, jint status) {
    auto msg = "onCharacteristicReadCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    obj->clear_flag_characteristicReadPending(characteristic_obj, value.bytes());
}

void BluetoothGattCallback::jni_onCharacteristicWriteCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj,
                                                              jint status) {
    auto msg = "onCharacteristicWriteCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    obj->clear_flag_characteristicWritePending(characteristic_obj);
}

void BluetoothGattCallback::jni_onDescriptorReadCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor_obj,
                                                         SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value, jint status) {
    auto msg = "onDescriptorReadCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    obj->clear_flag_descriptorReadPending(descriptor_obj, value.bytes());
}

void BluetoothGattCallback::jni_onDescriptorWriteCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor_obj,
                                                          jint status) {
    auto msg = "onDescriptorWriteCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    obj->clear_flag_descriptorWritePending(descriptor_obj);
}

void BluetoothGattCallback::jni_onMtuChangedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint mtu, jint status) {
    auto msg = "onMtuChangedCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
    obj->mtu = mtu;
}

void BluetoothGattCallback::jni_onPhyReadCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint tx_phy, jint rx_phy, jint status) {
    auto msg = "onPhyReadCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
}

void BluetoothGattCallback::jni_onPhyUpdateCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint tx_phy, jint rx_phy, jint status) {
    auto msg = "onPhyUpdateCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
}

void BluetoothGattCallback::jni_onReadRemoteRssiCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint rssi, jint status) {
    auto msg = "onReadRemoteRssiCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
}

void BluetoothGattCallback::jni_onReliableWriteCompletedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint status) {
    auto msg = "onReliableWriteCompletedCallback";
    SIMPLEBLE_LOG_INFO(msg);

    BluetoothGattCallback* obj = GET_CALLBACK_OBJECT_OR_RETURN(thiz_obj);
}

}  // namespace Bridge
}  // namespace Android
}  // namespace SimpleBLE

extern "C" {
// clang-format off

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onConnectionStateChangeCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint status, jint new_state) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, status, new_state]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onConnectionStateChangeCallback(thiz_obj, status, new_state);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onServicesDiscoveredCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onServicesDiscoveredCallback(thiz_obj, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onServiceChangedCallback(
    JNIEnv* env, jobject thiz, jobject gatt) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onServiceChangedCallback(thiz_obj);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onCharacteristicChangedCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject characteristic, jbyteArray value) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj(characteristic);
    SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value_obj(value);
    SimpleJNI::Runner::get().enqueue([thiz_obj, characteristic_obj, value_obj]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onCharacteristicChangedCallback(thiz_obj, characteristic_obj, value_obj);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onCharacteristicReadCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject characteristic, jbyteArray value, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj(characteristic);
    SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value_obj(value);
    SimpleJNI::Runner::get().enqueue([thiz_obj, characteristic_obj, value_obj, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onCharacteristicReadCallback(thiz_obj, characteristic_obj, value_obj, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onCharacteristicWriteCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject characteristic, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj(characteristic);
    SimpleJNI::Runner::get().enqueue([thiz_obj, characteristic_obj, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onCharacteristicWriteCallback(thiz_obj, characteristic_obj, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onDescriptorReadCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject descriptor, jbyteArray value, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor_obj(descriptor);
    SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value_obj(value);
    SimpleJNI::Runner::get().enqueue([thiz_obj, descriptor_obj, value_obj, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onDescriptorReadCallback(thiz_obj, descriptor_obj, value_obj, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onDescriptorWriteCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jobject descriptor, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor_obj(descriptor);
    SimpleJNI::Runner::get().enqueue([thiz_obj, descriptor_obj, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onDescriptorWriteCallback(thiz_obj, descriptor_obj, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onMtuChangedCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint mtu, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, mtu, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onMtuChangedCallback(thiz_obj, mtu, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onPhyReadCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint tx_phy, jint rx_phy, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, tx_phy, rx_phy, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onPhyReadCallback(thiz_obj, tx_phy, rx_phy, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onPhyUpdateCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint tx_phy, jint rx_phy, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, tx_phy, rx_phy, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onPhyUpdateCallback(thiz_obj, tx_phy, rx_phy, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onReadRemoteRssiCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint rssi, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, rssi, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onReadRemoteRssiCallback(thiz_obj, rssi, status);
    });
}

JNIEXPORT void JNICALL Java_org_simpleble_android_bridge_BluetoothGattCallback_onReliableWriteCompletedCallback(
    JNIEnv* env, jobject thiz, jobject gatt, jint status) {
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj(thiz);
    SimpleJNI::Runner::get().enqueue([thiz_obj, status]() {
        SimpleBLE::Android::Bridge::BluetoothGattCallback::jni_onReliableWriteCompletedCallback(thiz_obj, status);
    });
}

// clang-format on
}  // extern "C"