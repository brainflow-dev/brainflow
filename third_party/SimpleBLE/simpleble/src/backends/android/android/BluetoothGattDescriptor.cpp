#include "BluetoothGattDescriptor.h"
#include "UUID.h"

namespace SimpleBLE {
namespace Android {

JNI::Class BluetoothGattDescriptor::_cls;
jmethodID BluetoothGattDescriptor::_method_getUuid = nullptr;
jmethodID BluetoothGattDescriptor::_method_getValue = nullptr;
jmethodID BluetoothGattDescriptor::_method_setValue = nullptr;

const std::string BluetoothGattDescriptor::CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
const std::vector<uint8_t> BluetoothGattDescriptor::DISABLE_NOTIFICATION_VALUE = {0x00, 0x00};
const std::vector<uint8_t> BluetoothGattDescriptor::ENABLE_NOTIFICATION_VALUE = {0x01, 0x00};
const std::vector<uint8_t> BluetoothGattDescriptor::ENABLE_INDICATION_VALUE = {0x02, 0x00};

void BluetoothGattDescriptor::initialize() {
    JNI::Env env;

    if (_cls.get() == nullptr) {
        _cls = env.find_class("android/bluetooth/BluetoothGattDescriptor");
    }

    if (!_method_getUuid) {
        _method_getUuid = env->GetMethodID(_cls.get(), "getUuid", "()Ljava/util/UUID;");
    }

    if (!_method_getValue) {
        _method_getValue = env->GetMethodID(_cls.get(), "getValue", "()[B");
    }

    if (!_method_setValue) {
        _method_setValue = env->GetMethodID(_cls.get(), "setValue", "([B)Z");
    }
}

void BluetoothGattDescriptor::check_initialized() const {
    if (!_obj) throw std::runtime_error("BluetoothGattDescriptor is not initialized");
}

BluetoothGattDescriptor::BluetoothGattDescriptor() { initialize(); }

BluetoothGattDescriptor::BluetoothGattDescriptor(JNI::Object obj) : BluetoothGattDescriptor() { _obj = obj; }

std::string BluetoothGattDescriptor::getUuid() {
    check_initialized();

    JNI::Object uuidObj = _obj.call_object_method(_method_getUuid);
    if (!uuidObj) throw std::runtime_error("Failed to get UUID");

    return UUID(uuidObj).toString();
}

std::vector<uint8_t> BluetoothGattDescriptor::getValue() {
    check_initialized();

    return _obj.call_byte_array_method(_method_getValue);
}

bool BluetoothGattDescriptor::setValue(const std::vector<uint8_t> &value) {
    check_initialized();

    JNI::Env env;
    jbyteArray jbyteArray_obj = env->NewByteArray(value.size());
    env->SetByteArrayRegion(jbyteArray_obj, 0, value.size(), reinterpret_cast<const jbyte *>(value.data()));

    bool result = _obj.call_boolean_method(_method_setValue, jbyteArray_obj);

    env->DeleteLocalRef(jbyteArray_obj);
    return result;
}

}  // namespace Android
}  // namespace SimpleBLE