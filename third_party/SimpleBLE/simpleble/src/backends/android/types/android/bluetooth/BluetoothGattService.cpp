//
// Created by Kevin Dewald on 5/17/24.
//

#include "BluetoothGattService.h"
#include "types/java/util/UUID.h"
#include "types/java/util/List.h"

namespace SimpleBLE {
namespace Android {

SimpleJNI::GlobalRef<jclass> BluetoothGattService::_cls;
jmethodID BluetoothGattService::_method_addCharacteristic = nullptr;
jmethodID BluetoothGattService::_method_addService = nullptr;
jmethodID BluetoothGattService::_method_getCharacteristic = nullptr;
jmethodID BluetoothGattService::_method_getCharacteristics = nullptr;
jmethodID BluetoothGattService::_method_getIncludedServices = nullptr;
jmethodID BluetoothGattService::_method_getInstanceId = nullptr;
jmethodID BluetoothGattService::_method_getType = nullptr;
jmethodID BluetoothGattService::_method_getUuid = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor BluetoothGattService::descriptor{
    "android/bluetooth/BluetoothGattService", // Java class name
    &_cls,                                    // Where to store the jclass
    {                                         // Methods to preload
        {"addCharacteristic", "(Landroid/bluetooth/BluetoothGattCharacteristic;)Z", &_method_addCharacteristic},
        {"addService", "(Landroid/bluetooth/BluetoothGattService;)Z", &_method_addService},
        {"getCharacteristic", "(Ljava/util/UUID;)Landroid/bluetooth/BluetoothGattCharacteristic;", &_method_getCharacteristic},
        {"getCharacteristics", "()Ljava/util/List;", &_method_getCharacteristics},
        {"getIncludedServices", "()Ljava/util/List;", &_method_getIncludedServices},
        {"getInstanceId", "()I", &_method_getInstanceId},
        {"getType", "()I", &_method_getType},
        {"getUuid", "()Ljava/util/UUID;", &_method_getUuid}
    }};

const SimpleJNI::AutoRegister<BluetoothGattService> BluetoothGattService::registrar{&descriptor};

BluetoothGattService::BluetoothGattService() : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("BluetoothGattService JNI resources not preloaded. Ensure SimpleJNI::Registrar::preload() is called.");
    }
}

BluetoothGattService::BluetoothGattService(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

// bool BluetoothGattService::addCharacteristic(BluetoothGattCharacteristic characteristic) {
//     return _obj.call_boolean_method(_method_addCharacteristic, characteristic.getObject());
// }
//
// bool BluetoothGattService::addService(BluetoothGattService service) {
//     return _obj.call_boolean_method(_method_addService, service.getObject());
// }
//
// BluetoothGattCharacteristic BluetoothGattService::getCharacteristic(std::string uuid) {
//     JNI::Env env;
//     SimpleJNI::Object<SimpleJNI::LocalRef, jobject> charObj = _obj.call_object_method(_method_getCharacteristic, env->NewStringUTF(uuid.c_str()));
//     return BluetoothGattCharacteristic(charObj.to_global());
// }
//
std::vector<BluetoothGattCharacteristic> BluetoothGattService::getCharacteristics() {
    if (!_obj) throw std::runtime_error("BluetoothGattService is not initialized");

    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> characteristics_obj = _obj.call_object_method(_method_getCharacteristics);
    if (!characteristics_obj) throw std::runtime_error("Failed to get characteristics");

    std::vector<BluetoothGattCharacteristic> result;
    List list(characteristics_obj.to_global());
    Iterator iterator = list.iterator();
    while (iterator.hasNext()) {
        SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> characteristic = iterator.next();

        if (!characteristic) continue;  // TODO: Should we throw an error here?
        result.push_back(BluetoothGattCharacteristic(characteristic));
    }

    return result;
}
//
// std::vector<BluetoothGattService> BluetoothGattService::getIncludedServices() {
//    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> listObj = _obj.call_object_method(_method_getIncludedServices);
//    // TODO: Implement conversion from list to vector of BluetoothGattService
//    return std::vector<BluetoothGattService>();
//}

int BluetoothGattService::getInstanceId() {
    if (!_obj) throw std::runtime_error("BluetoothGattService is not initialized");
    return _obj.call_int_method(_method_getInstanceId);
}

int BluetoothGattService::getType() {
    if (!_obj) throw std::runtime_error("BluetoothGattService is not initialized");
    return _obj.call_int_method(_method_getType);
}

std::string BluetoothGattService::getUuid() {
    if (!_obj) throw std::runtime_error("BluetoothGattService is not initialized");

    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> uuidObj = _obj.call_object_method(_method_getUuid);
    if (!uuidObj) throw std::runtime_error("Failed to get UUID");

    return UUID(uuidObj.to_global()).toString();
}

}  // namespace Android
}  // namespace SimpleBLE