#include "BluetoothGattDescriptor.h"
#include "types/java/util/UUID.h"

namespace SimpleBLE {
namespace Android {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> BluetoothGattDescriptor::_cls;
jmethodID BluetoothGattDescriptor::_method_getUuid = nullptr;
jmethodID BluetoothGattDescriptor::_method_getValue = nullptr;
jmethodID BluetoothGattDescriptor::_method_setValue = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor BluetoothGattDescriptor::descriptor{
    "android/bluetooth/BluetoothGattDescriptor", // Java class name
    &_cls,                                       // Where to store the jclass
    {                                            // Methods to preload
        {"getUuid", "()Ljava/util/UUID;", &_method_getUuid},
        {"getValue", "()[B", &_method_getValue},
        {"setValue", "([B)Z", &_method_setValue}
    }};

const SimpleJNI::AutoRegister<BluetoothGattDescriptor> BluetoothGattDescriptor::registrar{&descriptor};

const std::string BluetoothGattDescriptor::CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
const std::vector<uint8_t> BluetoothGattDescriptor::DISABLE_NOTIFICATION_VALUE = {0x00, 0x00};
const std::vector<uint8_t> BluetoothGattDescriptor::ENABLE_NOTIFICATION_VALUE = {0x01, 0x00};
const std::vector<uint8_t> BluetoothGattDescriptor::ENABLE_INDICATION_VALUE = {0x02, 0x00};

BluetoothGattDescriptor::BluetoothGattDescriptor() : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("BluetoothGattDescriptor JNI resources not preloaded. Ensure SimpleJNI::Registrar::preload() is called.");
    }
}

BluetoothGattDescriptor::BluetoothGattDescriptor(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

std::string BluetoothGattDescriptor::getUuid() {
    if (!_obj) throw std::runtime_error("BluetoothGattDescriptor is not initialized");

    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> uuidObj = _obj.call_object_method(_method_getUuid);
    if (!uuidObj) throw std::runtime_error("Failed to get UUID");

    return UUID(uuidObj.to_global()).toString();
}

std::vector<uint8_t> BluetoothGattDescriptor::getValue() {
    if (!_obj) throw std::runtime_error("BluetoothGattDescriptor is not initialized");

    return _obj.call_byte_array_method(_method_getValue);
}

bool BluetoothGattDescriptor::setValue(const std::vector<uint8_t>& value) {
    if (!_obj) throw std::runtime_error("BluetoothGattDescriptor is not initialized");

    SimpleJNI::Env env;
    jbyteArray jbyteArray_obj = env->NewByteArray(value.size());
    env->SetByteArrayRegion(jbyteArray_obj, 0, value.size(), reinterpret_cast<const jbyte*>(value.data()));

    bool result = _obj.call_boolean_method(_method_setValue, jbyteArray_obj);

    env->DeleteLocalRef(jbyteArray_obj);
    return result;
}

}  // namespace Android
}  // namespace SimpleBLE