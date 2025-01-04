#include "BluetoothDevice.h"

namespace SimpleBLE {
namespace Android {

JNI::Class BluetoothDevice::_cls;
jmethodID BluetoothDevice::_method_getAddress;
jmethodID BluetoothDevice::_method_getName;
jmethodID BluetoothDevice::_method_connectGatt;

void BluetoothDevice::initialize() {
    JNI::Env env;

    if (_cls.get() == nullptr) {
        _cls = env.find_class("android/bluetooth/BluetoothDevice");
    }

    if (!_method_getAddress) {
        _method_getAddress = env->GetMethodID(_cls.get(), "getAddress", "()Ljava/lang/String;");
    }

    if (!_method_getName) {
        _method_getName = env->GetMethodID(_cls.get(), "getName", "()Ljava/lang/String;");
    }

    if (!_method_connectGatt) {
        _method_connectGatt = env->GetMethodID(
            _cls.get(), "connectGatt",
            "(Landroid/content/Context;ZLandroid/bluetooth/BluetoothGattCallback;)Landroid/bluetooth/BluetoothGatt;");
    }
}

BluetoothDevice::BluetoothDevice(JNI::Object obj) : _obj(obj) {
    initialize();
};

void BluetoothDevice::check_initialized() const {
    if (!_obj) throw std::runtime_error("BluetoothDevice is not initialized");
}

std::string BluetoothDevice::getAddress() {
    check_initialized();
    return _obj.call_string_method(_method_getAddress);
}

std::string BluetoothDevice::getName() {
    check_initialized();
    return _obj.call_string_method(_method_getName);
}

BluetoothGatt BluetoothDevice::connectGatt(bool autoConnect, Bridge::BluetoothGattCallback& callback) {
    check_initialized();
    return BluetoothGatt(_obj.call_object_method(_method_connectGatt, nullptr, autoConnect, callback.get()));
}

}  // namespace Android
}  // namespace SimpleBLE
