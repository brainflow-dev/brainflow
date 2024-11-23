//
// Created by Kevin Dewald on 5/17/24.
//

#include "BluetoothGattCharacteristic.h"
#include "UUID.h"
#include "jni/Types.h"

namespace SimpleBLE {
namespace Android {

JNI::Class BluetoothGattCharacteristic::_cls;
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

void BluetoothGattCharacteristic::initialize() {
    JNI::Env env;

    if (_cls.get() == nullptr) {
        _cls = env.find_class("android/bluetooth/BluetoothGattCharacteristic");
    }

    if (!_method_addDescriptor) {
        _method_addDescriptor = env->GetMethodID(_cls.get(), "addDescriptor",
                                                 "(Landroid/bluetooth/BluetoothGattDescriptor;)Z");
    }

    if (!_method_getDescriptor) {
        _method_getDescriptor = env->GetMethodID(_cls.get(), "getDescriptor",
                                                 "(Ljava/util/UUID;)Landroid/bluetooth/BluetoothGattDescriptor;");
    }

    if (!_method_getDescriptors) {
        _method_getDescriptors = env->GetMethodID(_cls.get(), "getDescriptors", "()Ljava/util/List;");
    }

    if (!_method_getInstanceId) {
        _method_getInstanceId = env->GetMethodID(_cls.get(), "getInstanceId", "()I");
    }

    if (!_method_getPermissions) {
        _method_getPermissions = env->GetMethodID(_cls.get(), "getPermissions", "()I");
    }

    if (!_method_getProperties) {
        _method_getProperties = env->GetMethodID(_cls.get(), "getProperties", "()I");
    }

    if (!_method_getService) {
        _method_getService = env->GetMethodID(_cls.get(), "getService", "()Landroid/bluetooth/BluetoothGattService;");
    }

    if (!_method_getUuid) {
        _method_getUuid = env->GetMethodID(_cls.get(), "getUuid", "()Ljava/util/UUID;");
    }

    if (!_method_getWriteType) {
        _method_getWriteType = env->GetMethodID(_cls.get(), "getWriteType", "()I");
    }

    if (!_method_setWriteType) {
        _method_setWriteType = env->GetMethodID(_cls.get(), "setWriteType", "(I)V");
    }

    if (!_method_setValue) {
        _method_setValue = env->GetMethodID(_cls.get(), "setValue", "([B)Z");
    }
}

void BluetoothGattCharacteristic::check_initialized() const {
    if (!_obj) throw std::runtime_error("BluetoothGattCharacteristic is not initialized");
}

BluetoothGattCharacteristic::BluetoothGattCharacteristic() { initialize(); }

BluetoothGattCharacteristic::BluetoothGattCharacteristic(JNI::Object obj) : BluetoothGattCharacteristic() {
    _obj = obj;
}

// bool BluetoothGattCharacteristic::addDescriptor(BluetoothGattDescriptor descriptor) {
//     return _obj.call_boolean_method(_method_addDescriptor, descriptor.getObject());
// }
//
// BluetoothGattDescriptor BluetoothGattCharacteristic::getDescriptor(std::string uuid) {
//     JNI::Env env;
//     JNI::Object descObj = _obj.call_object_method(_method_getDescriptor, env->NewStringUTF(uuid.c_str()));
//     return BluetoothGattDescriptor(descObj);
// }
//
std::vector<BluetoothGattDescriptor> BluetoothGattCharacteristic::getDescriptors() {
    check_initialized();

    JNI::Object descriptors = _obj.call_object_method(_method_getDescriptors);
    if (!descriptors) throw std::runtime_error("Failed to get descriptors");

    std::vector<BluetoothGattDescriptor> result;
    JNI::Object iterator = descriptors.call_object_method("iterator", "()Ljava/util/Iterator;");
    while (iterator.call_boolean_method("hasNext", "()Z")) {
        JNI::Object descriptor = iterator.call_object_method("next", "()Ljava/lang/Object;");

        if (!descriptor) continue;
        result.push_back(BluetoothGattDescriptor(descriptor));
    }

    return result;
}

int BluetoothGattCharacteristic::getInstanceId() {
    check_initialized();
    return _obj.call_int_method(_method_getInstanceId);
}

int BluetoothGattCharacteristic::getPermissions() {
    check_initialized();
    return _obj.call_int_method(_method_getPermissions);
}

int BluetoothGattCharacteristic::getProperties() {
    check_initialized();
    return _obj.call_int_method(_method_getProperties);
}

std::string BluetoothGattCharacteristic::getUuid() {
    check_initialized();

    JNI::Object uuidObj = _obj.call_object_method(_method_getUuid);
    if (!uuidObj) throw std::runtime_error("Failed to get UUID");

    return UUID(uuidObj).toString();
}

int BluetoothGattCharacteristic::getWriteType() {
    check_initialized();
    return _obj.call_int_method(_method_getWriteType);
}

void BluetoothGattCharacteristic::setWriteType(int writeType) {
    check_initialized();
    _obj.call_void_method(_method_setWriteType, writeType);
}

bool BluetoothGattCharacteristic::setValue(const std::vector<uint8_t>& value) {
    check_initialized();

    JNI::Env env;
    jbyteArray array = JNI::Types::toJByteArray(value);

    bool result = _obj.call_boolean_method(_method_setValue, array);
    env->DeleteLocalRef(array);
    return result;
}

}  // namespace Android
}  // namespace SimpleBLE