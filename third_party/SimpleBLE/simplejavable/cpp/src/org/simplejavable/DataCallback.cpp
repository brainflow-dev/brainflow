#include "DataCallback.h"

namespace Org {
namespace SimpleJavaBLE {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> DataCallback::_cls;
jmethodID DataCallback::_method_on_data_received = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor DataCallback::descriptor{
    "org/simplejavable/Peripheral$DataCallback",  // Java interface name (inner class notation)
    &_cls,                                        // Where to store the jclass
    {                                             // Methods to preload
     {"onDataReceived", "([B)V", &_method_on_data_received}}};

// Define the AutoRegister instance
const SimpleJNI::AutoRegister<DataCallback> DataCallback::registrar{&descriptor};

DataCallback::DataCallback(jobject obj) : _obj(obj, _cls.get()) {
    if (!_cls.get()) {
        throw std::runtime_error("DataCallback JNI resources not preloaded");
    }
}

void DataCallback::on_data_received(jbyteArray data) {
    if (_obj.is_valid()) {
        _obj.to_local().call_void_method(_method_on_data_received, data);
    }
}

}  // namespace SimpleJavaBLE
}  // namespace Org