//
// Created by Kevin Dewald on 5/17/24.
//

#include "BluetoothGattService.h"
#include "UUID.h"

namespace SimpleBLE {
namespace Android {

JNI::Class BluetoothGattService::_cls;
jmethodID BluetoothGattService::_method_addCharacteristic = nullptr;
jmethodID BluetoothGattService::_method_addService = nullptr;
jmethodID BluetoothGattService::_method_getCharacteristic = nullptr;
jmethodID BluetoothGattService::_method_getCharacteristics = nullptr;
jmethodID BluetoothGattService::_method_getIncludedServices = nullptr;
jmethodID BluetoothGattService::_method_getInstanceId = nullptr;
jmethodID BluetoothGattService::_method_getType = nullptr;
jmethodID BluetoothGattService::_method_getUuid = nullptr;

void BluetoothGattService::initialize() {
    JNI::Env env;

    if (_cls.get() == nullptr) {
        _cls = env.find_class("android/bluetooth/BluetoothGattService");
    }

    if (!_method_addCharacteristic) {
        _method_addCharacteristic = env->GetMethodID(_cls.get(), "addCharacteristic",
                                                     "(Landroid/bluetooth/BluetoothGattCharacteristic;)Z");
    }

    if (!_method_addService) {
        _method_addService = env->GetMethodID(_cls.get(), "addService", "(Landroid/bluetooth/BluetoothGattService;)Z");
    }

    if (!_method_getCharacteristic) {
        _method_getCharacteristic = env->GetMethodID(
            _cls.get(), "getCharacteristic", "(Ljava/util/UUID;)Landroid/bluetooth/BluetoothGattCharacteristic;");
    }

    if (!_method_getCharacteristics) {
        _method_getCharacteristics = env->GetMethodID(_cls.get(), "getCharacteristics", "()Ljava/util/List;");
    }

    if (!_method_getIncludedServices) {
        _method_getIncludedServices = env->GetMethodID(_cls.get(), "getIncludedServices", "()Ljava/util/List;");
    }

    if (!_method_getInstanceId) {
        _method_getInstanceId = env->GetMethodID(_cls.get(), "getInstanceId", "()I");
    }

    if (!_method_getType) {
        _method_getType = env->GetMethodID(_cls.get(), "getType", "()I");
    }

    if (!_method_getUuid) {
        _method_getUuid = env->GetMethodID(_cls.get(), "getUuid", "()Ljava/util/UUID;");
    }
}

void BluetoothGattService::check_initialized() const {
    if (!_obj) throw std::runtime_error("BluetoothGattService is not initialized");
}

BluetoothGattService::BluetoothGattService() { initialize(); }


BluetoothGattService::BluetoothGattService(JNI::Object obj) : BluetoothGattService() {
    _obj = obj;
}

//bool BluetoothGattService::addCharacteristic(BluetoothGattCharacteristic characteristic) {
//    return _obj.call_boolean_method(_method_addCharacteristic, characteristic.getObject());
//}
//
//bool BluetoothGattService::addService(BluetoothGattService service) {
//    return _obj.call_boolean_method(_method_addService, service.getObject());
//}
//
//BluetoothGattCharacteristic BluetoothGattService::getCharacteristic(std::string uuid) {
//    JNI::Env env;
//    JNI::Object charObj = _obj.call_object_method(_method_getCharacteristic, env->NewStringUTF(uuid.c_str()));
//    return BluetoothGattCharacteristic(charObj);
//}
//
std::vector<BluetoothGattCharacteristic> BluetoothGattService::getCharacteristics() {
    check_initialized();

    JNI::Object characteristics = _obj.call_object_method(_method_getCharacteristics);
    if (!characteristics) throw std::runtime_error("Failed to get characteristics");

    std::vector<BluetoothGattCharacteristic> result;
    JNI::Object iterator = characteristics.call_object_method("iterator", "()Ljava/util/Iterator;");
    while (iterator.call_boolean_method("hasNext", "()Z")) {
        JNI::Object characteristic = iterator.call_object_method("next", "()Ljava/lang/Object;");

        if (!characteristic) continue; // TODO: Should we throw an error here?
        result.push_back(BluetoothGattCharacteristic(characteristic));
    }

    return result;
}
//
//std::vector<BluetoothGattService> BluetoothGattService::getIncludedServices() {
//    JNI::Env env;
//    JNI::Object listObj = _obj.call_object_method(_method_getIncludedServices);
//    return JNI::convert_list<BluetoothGattService>(listObj);
//}

int BluetoothGattService::getInstanceId() {
    check_initialized();
    return _obj.call_int_method(_method_getInstanceId);
}

int BluetoothGattService::getType() {
    check_initialized();
    return _obj.call_int_method(_method_getType);
}

std::string BluetoothGattService::getUuid() {
    check_initialized();

    JNI::Object uuidObj = _obj.call_object_method(_method_getUuid);
    if (!uuidObj) throw std::runtime_error("Failed to get UUID");

    return UUID(uuidObj).toString();
}

}  // namespace Android
}  // namespace SimpleBLE