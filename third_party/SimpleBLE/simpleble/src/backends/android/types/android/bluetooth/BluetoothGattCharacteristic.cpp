//
// Created by Kevin Dewald on 5/17/24.
//

#include "BluetoothGattCharacteristic.h"
#include "types/java/util/UUID.h"
#include "types/java/util/List.h"

namespace SimpleBLE {
namespace Android {

// Static JNI resources
SimpleJNI::GlobalRef<jclass> BluetoothGattCharacteristic::_cls;
jmethodID BluetoothGattCharacteristic::_method_addDescriptor = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getDescriptor = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getDescriptors = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getInstanceId = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getPermissions = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getProperties = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getService = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getUuid = nullptr;
jmethodID BluetoothGattCharacteristic::_method_getWriteType = nullptr;
jmethodID BluetoothGattCharacteristic::_method_setWriteType = nullptr;
jmethodID BluetoothGattCharacteristic::_method_setValue = nullptr;

// JNI descriptor for auto-registration
const SimpleJNI::JNIDescriptor BluetoothGattCharacteristic::descriptor{
    "android/bluetooth/BluetoothGattCharacteristic", // Java class name
    &_cls,                                           // Where to store the jclass
    {                                                // Methods to preload
        {"addDescriptor", "(Landroid/bluetooth/BluetoothGattDescriptor;)Z", &_method_addDescriptor},
        {"getDescriptor", "(Ljava/util/UUID;)Landroid/bluetooth/BluetoothGattDescriptor;", &_method_getDescriptor},
        {"getDescriptors", "()Ljava/util/List;", &_method_getDescriptors},
        {"getInstanceId", "()I", &_method_getInstanceId},
        {"getPermissions", "()I", &_method_getPermissions},
        {"getProperties", "()I", &_method_getProperties},
        {"getService", "()Landroid/bluetooth/BluetoothGattService;", &_method_getService},
        {"getUuid", "()Ljava/util/UUID;", &_method_getUuid},
        {"getWriteType", "()I", &_method_getWriteType},
        {"setWriteType", "(I)V", &_method_setWriteType},
        {"setValue", "([B)Z", &_method_setValue}
    }};

// Auto-register the descriptor
const SimpleJNI::AutoRegister<BluetoothGattCharacteristic> BluetoothGattCharacteristic::registrar{&descriptor};

BluetoothGattCharacteristic::BluetoothGattCharacteristic() : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("BluetoothGattCharacteristic JNI resources not preloaded. Ensure SimpleJNI::Registrar::preload() is called.");
    }

    SimpleJNI::Env env;
    jobject local_obj = env->NewObject(_cls.get(), env->GetMethodID(_cls.get(), "<init>", "()V"));
    if (local_obj == nullptr) {
        throw std::runtime_error("Failed to create BluetoothGattCharacteristic Java instance");
    }

    _obj = SimpleJNI::Object<SimpleJNI::GlobalRef, jobject>(local_obj);
    env->DeleteLocalRef(local_obj);
}

BluetoothGattCharacteristic::BluetoothGattCharacteristic(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

// bool BluetoothGattCharacteristic::addDescriptor(BluetoothGattDescriptor descriptor) {
//     return _obj.call_boolean_method(_method_addDescriptor, descriptor.getObject());
// }
//
// BluetoothGattDescriptor BluetoothGattCharacteristic::getDescriptor(std::string uuid) {
//     JNI::Env env;
//     SimpleJNI::Object<SimpleJNI::LocalRef, jobject> descObj = _obj.call_object_method(_method_getDescriptor, env->NewStringUTF(uuid.c_str()));
//     return BluetoothGattDescriptor(descObj);
// }
//
std::vector<BluetoothGattDescriptor> BluetoothGattCharacteristic::getDescriptors() {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");

    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> descriptors_obj = _obj.call_object_method(_method_getDescriptors);
    if (!descriptors_obj) throw std::runtime_error("Failed to get descriptors");

    std::vector<BluetoothGattDescriptor> result;
    List list(descriptors_obj.to_global());
    Iterator iterator = list.iterator();
    while (iterator.hasNext()) {
        SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> descriptor = iterator.next();

        if (!descriptor) continue;
        result.push_back(BluetoothGattDescriptor(descriptor));
    }

    return result;
}

int BluetoothGattCharacteristic::getInstanceId() {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");
    return _obj.call_int_method(_method_getInstanceId);
}

int BluetoothGattCharacteristic::getPermissions() {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");
    return _obj.call_int_method(_method_getPermissions);
}

int BluetoothGattCharacteristic::getProperties() {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");
    return _obj.call_int_method(_method_getProperties);
}

std::string BluetoothGattCharacteristic::getUuid() {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");

    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> uuidObj = _obj.call_object_method(_method_getUuid);
    if (!uuidObj) throw std::runtime_error("Failed to get UUID");

    return UUID(uuidObj.to_global()).toString();
}

int BluetoothGattCharacteristic::getWriteType() {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");
    return _obj.call_int_method(_method_getWriteType);
}

void BluetoothGattCharacteristic::setWriteType(int writeType) {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");
    _obj.call_void_method(_method_setWriteType, writeType);
}

bool BluetoothGattCharacteristic::setValue(const std::vector<uint8_t>& value) {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");

    SimpleJNI::ByteArray<SimpleJNI::LocalRef> jni_array_wrapper(value);

    if (!jni_array_wrapper) {
        throw std::runtime_error("Failed to create Java byte array for setValue using SimpleJNI::ByteArray");
    }

    bool success = _obj.call_boolean_method(_method_setValue, jni_array_wrapper.get());

    return success;
}

}  // namespace Android
}  // namespace SimpleBLE