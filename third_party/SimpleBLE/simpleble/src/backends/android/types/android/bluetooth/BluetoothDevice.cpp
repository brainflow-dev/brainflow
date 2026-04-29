#include "BluetoothDevice.h"

namespace SimpleBLE {
namespace Android {

// Static constants
const int BluetoothDevice::BOND_NONE;
const int BluetoothDevice::BOND_BONDING;
const int BluetoothDevice::BOND_BONDED;

const int BluetoothDevice::ADDRESS_TYPE_ANONYMOUS;
const int BluetoothDevice::ADDRESS_TYPE_PUBLIC;
const int BluetoothDevice::ADDRESS_TYPE_RANDOM;

const int BluetoothDevice::TRANSPORT_AUTO;
const int BluetoothDevice::TRANSPORT_BREDR;
const int BluetoothDevice::TRANSPORT_LE;

// Static JNI resources
SimpleJNI::GlobalRef<jclass> BluetoothDevice::_cls;
jmethodID BluetoothDevice::_method_getAddress;
jmethodID BluetoothDevice::_method_getName;
jmethodID BluetoothDevice::_method_getBondState;
jmethodID BluetoothDevice::_method_connectGatt;

// JNI descriptor and registrar
const SimpleJNI::JNIDescriptor BluetoothDevice::descriptor{
    "android/bluetooth/BluetoothDevice", // Java class name
    &_cls,                               // Where to store the jclass
    {                                    // Methods to preload
        {"getAddress", "()Ljava/lang/String;", &_method_getAddress},
        {"getName", "()Ljava/lang/String;", &_method_getName},
        {"getBondState", "()I", &_method_getBondState},
        {"connectGatt", "(Landroid/content/Context;ZLandroid/bluetooth/BluetoothGattCallback;I)Landroid/bluetooth/BluetoothGatt;", &_method_connectGatt}
    }
};

const SimpleJNI::AutoRegister<BluetoothDevice> BluetoothDevice::registrar{&descriptor};

BluetoothDevice::BluetoothDevice(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {
    if (!_cls.get()) {
        throw std::runtime_error("BluetoothDevice JNI resources not preloaded. Ensure SimpleJNI::Registrar::preload() is called.");
    }
}

std::string BluetoothDevice::getAddress() {
    if (!_obj) throw std::runtime_error("BluetoothDevice is not initialized");
    return _obj.call_string_method(_method_getAddress);
}

std::string BluetoothDevice::getName() {
    if (!_obj) throw std::runtime_error("BluetoothDevice is not initialized");
    return _obj.call_string_method(_method_getName);
}

int BluetoothDevice::getBondState() {
    if (!_obj) throw std::runtime_error("BluetoothDevice is not initialized");
    return _obj.call_int_method(_method_getBondState);
}

BluetoothGatt BluetoothDevice::connectGatt(bool autoConnect, Bridge::BluetoothGattCallback& callback) {
    if (!_obj) throw std::runtime_error("BluetoothDevice is not initialized");
    return BluetoothGatt(
        _obj.call_object_method(_method_connectGatt, nullptr, autoConnect, callback.get(), BluetoothDevice::TRANSPORT_LE));
}

}  // namespace Android
}  // namespace SimpleBLE
