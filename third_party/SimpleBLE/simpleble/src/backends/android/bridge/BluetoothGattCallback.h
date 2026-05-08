#pragma once

#include <atomic>
#include <condition_variable>
#include <kvn_safe_callback.hpp>
#include <kvn_safe_map.hpp>
#include <map>
#include <mutex>
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace SimpleBLE {
namespace Android {
namespace Bridge {

class BluetoothGattCallback {
  public:
    BluetoothGattCallback();
    virtual ~BluetoothGattCallback();
    jobject get() const { return _obj.get(); }  // TODO: Remove once nothing uses this

    void set_callback_onConnectionStateChange(std::function<void(bool)> callback);
    void set_callback_onServicesDiscovered(std::function<void(void)> callback);

    void set_callback_onCharacteristicChanged(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic,
                                              std::function<void(std::vector<uint8_t> value)> callback);
    void clear_callback_onCharacteristicChanged(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic);

    void set_flag_characteristicWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic);
    void clear_flag_characteristicWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic);
    void wait_flag_characteristicWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic);

    void set_flag_characteristicReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic);
    void clear_flag_characteristicReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic, std::vector<uint8_t> value);
    std::vector<uint8_t> wait_flag_characteristicReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic);

    void set_flag_descriptorWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor);
    void clear_flag_descriptorWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor);
    void wait_flag_descriptorWritePending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor);

    void set_flag_descriptorReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor);
    void clear_flag_descriptorReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor, std::vector<uint8_t> value);
    std::vector<uint8_t> wait_flag_descriptorReadPending(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor);

    bool connected;
    bool services_discovered;
    uint16_t mtu;

    // Not for public use
    // clang-format off
    static void jni_onConnectionStateChangeCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint status, jint new_state);
    static void jni_onServicesDiscoveredCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint status);
    static void jni_onServiceChangedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj);

    static void jni_onCharacteristicChangedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj, SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value);
    static void jni_onCharacteristicReadCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj, SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value, jint status);
    static void jni_onCharacteristicWriteCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic_obj, jint status);

    static void jni_onDescriptorReadCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor_obj, SimpleJNI::ByteArray<SimpleJNI::GlobalRef> value, jint status);
    static void jni_onDescriptorWriteCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor_obj, jint status);

    static void jni_onMtuChangedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint mtu, jint status);
    static void jni_onPhyReadCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint txPhy, jint rxPhy, jint status);
    static void jni_onPhyUpdateCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint txPhy, jint rxPhy, jint status);
    static void jni_onReadRemoteRssiCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint rssi, jint status);
    static void jni_onReliableWriteCompletedCallback(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> thiz_obj, jint status);
    // clang-format on

  private:
    struct FlagData {
        bool flag = false;
        std::condition_variable cv;
        std::mutex mtx;
        std::vector<uint8_t> value;
    };

    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _constructor;

    static kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, BluetoothGattCallback*, SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>> _map;

    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    kvn::safe_callback<void(bool)> _callback_onConnectionStateChange;
    kvn::safe_callback<void()> _callback_onServicesDiscovered;

    kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, kvn::safe_callback<void(std::vector<uint8_t>)>, SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>>
        _callback_onCharacteristicChanged;

    kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, FlagData, SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>> _flag_characteristicWritePending;
    kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, FlagData, SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>> _flag_characteristicReadPending;
    kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, FlagData, SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>> _flag_descriptorWritePending;
    kvn::safe_map<SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>, FlagData, SimpleJNI::ObjectComparator<SimpleJNI::GlobalRef, jobject>> _flag_descriptorReadPending;

    // Static JNI resources managed by Registrar
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<BluetoothGattCallback> registrar;
};

}  // namespace Bridge
}  // namespace Android
}  // namespace SimpleBLE