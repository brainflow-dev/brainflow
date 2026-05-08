#include "BluetoothGatt.h"
#include <android/log.h>
#include <fmt/core.h>

#include "types/java/util/List.h"

namespace SimpleBLE {
namespace Android {

SimpleJNI::GlobalRef<jclass> BluetoothGatt::_cls;
jmethodID BluetoothGatt::_method_close = nullptr;
jmethodID BluetoothGatt::_method_connect = nullptr;
jmethodID BluetoothGatt::_method_disconnect = nullptr;
jmethodID BluetoothGatt::_method_discoverServices = nullptr;
jmethodID BluetoothGatt::_method_getServices = nullptr;
jmethodID BluetoothGatt::_method_readCharacteristic = nullptr;
jmethodID BluetoothGatt::_method_readDescriptor = nullptr;
jmethodID BluetoothGatt::_method_setCharacteristicNotification = nullptr;
jmethodID BluetoothGatt::_method_writeCharacteristic = nullptr;
jmethodID BluetoothGatt::_method_writeDescriptor = nullptr;
jmethodID BluetoothGatt::_method_requestConnectionPriority = nullptr;
// Define the JNI descriptor
const SimpleJNI::JNIDescriptor BluetoothGatt::descriptor{
    "android/bluetooth/BluetoothGatt", // Java class name
    &_cls,                             // Where to store the jclass
    {                                  // Methods to preload
        {"close", "()V", &_method_close},
        {"connect", "()Z", &_method_connect},
        {"disconnect", "()V", &_method_disconnect},
        {"discoverServices", "()Z", &_method_discoverServices},
        {"getServices", "()Ljava/util/List;", &_method_getServices},
        {"readCharacteristic", "(Landroid/bluetooth/BluetoothGattCharacteristic;)Z", &_method_readCharacteristic},
        {"readDescriptor", "(Landroid/bluetooth/BluetoothGattDescriptor;)Z", &_method_readDescriptor},
        {"setCharacteristicNotification", "(Landroid/bluetooth/BluetoothGattCharacteristic;Z)Z", &_method_setCharacteristicNotification},
        {"writeCharacteristic", "(Landroid/bluetooth/BluetoothGattCharacteristic;)Z", &_method_writeCharacteristic},
        {"writeDescriptor", "(Landroid/bluetooth/BluetoothGattDescriptor;)Z", &_method_writeDescriptor},
        {"requestConnectionPriority", "(I)Z", &_method_requestConnectionPriority}
    }};

const SimpleJNI::AutoRegister<BluetoothGatt> BluetoothGatt::registrar{&descriptor};

BluetoothGatt::BluetoothGatt() {}

BluetoothGatt::BluetoothGatt(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

void BluetoothGatt::close() {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    _obj.call_void_method(_method_close);
}

bool BluetoothGatt::connect() {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_connect);
}

void BluetoothGatt::disconnect() {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    _obj.call_void_method(_method_disconnect);
}

bool BluetoothGatt::discoverServices() {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_discoverServices);
}

std::vector<BluetoothGattService> BluetoothGatt::getServices() {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> services_obj = _obj.call_object_method(_method_getServices);
    if (!services_obj) return std::vector<BluetoothGattService>();

    std::vector<BluetoothGattService> result;
    // Assuming JNI::Types::List and related classes are adapted to SimpleJNI namespace
    // This part might need further adjustment based on how List and Iterator are implemented in SimpleJNI
    List list(services_obj.to_global());
    Iterator iterator = list.iterator();
    while (iterator.hasNext()) {
        SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> service = iterator.next();
        if (!service) continue;
        result.push_back(BluetoothGattService(service));
    }
    return result;
}

bool BluetoothGatt::readCharacteristic(BluetoothGattCharacteristic characteristic) {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_readCharacteristic, characteristic.getObject().get());
}

bool BluetoothGatt::readDescriptor(BluetoothGattDescriptor descriptor) {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_readDescriptor, descriptor.getObject().get());
}

bool BluetoothGatt::setCharacteristicNotification(BluetoothGattCharacteristic characteristic, bool enable) {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_setCharacteristicNotification, characteristic.getObject().get(), enable);
}

bool BluetoothGatt::writeCharacteristic(BluetoothGattCharacteristic characteristic) {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_writeCharacteristic, characteristic.getObject().get());
}

bool BluetoothGatt::writeDescriptor(BluetoothGattDescriptor descriptor) {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_writeDescriptor, descriptor.getObject().get());
}

bool BluetoothGatt::requestConnectionPriority(int connectionPriority) {
    if (!_obj) throw std::runtime_error("BluetoothGatt is not initialized");
    return _obj.call_boolean_method(_method_requestConnectionPriority, connectionPriority);
}

}  // namespace Android
}  // namespace SimpleBLE
