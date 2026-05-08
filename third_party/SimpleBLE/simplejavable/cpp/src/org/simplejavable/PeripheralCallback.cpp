#include "PeripheralCallback.h"

namespace Org {
namespace SimpleJavaBLE {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> PeripheralCallback::_cls;
jmethodID PeripheralCallback::_method_on_connected = nullptr;
jmethodID PeripheralCallback::_method_on_disconnected = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor PeripheralCallback::descriptor{
    "org/simplejavable/Peripheral$Callback",  // Java interface name (inner class notation)
    &_cls,                                    // Where to store the jclass
    {                                         // Methods to preload
     {"onConnected", "()V", &_method_on_connected},
     {"onDisconnected", "()V", &_method_on_disconnected}}};

// Define the AutoRegister instance
const SimpleJNI::AutoRegister<PeripheralCallback> PeripheralCallback::registrar{&descriptor};

PeripheralCallback::PeripheralCallback(jobject obj) : _obj(obj, _cls.get()) {
    if (!_cls.get()) {
        throw std::runtime_error("PeripheralCallback JNI resources not preloaded");
    }
}

void PeripheralCallback::on_connected() {
    if (_obj.is_valid()) {
        _obj.to_local().call_void_method(_method_on_connected);
    }
}

void PeripheralCallback::on_disconnected() {
    if (_obj.is_valid()) {
        _obj.to_local().call_void_method(_method_on_disconnected);
    }
}

}  // namespace SimpleJavaBLE
}  // namespace Org