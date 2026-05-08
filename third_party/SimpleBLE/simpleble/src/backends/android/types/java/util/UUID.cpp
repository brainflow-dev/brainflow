#include "UUID.h"

namespace SimpleBLE {
namespace Android {

SimpleJNI::GlobalRef<jclass> UUID::_cls;
jmethodID UUID::_method_toString = nullptr;

const SimpleJNI::JNIDescriptor UUID::descriptor{"java/util/UUID",  // Java class name
                                                &_cls,             // Pointer to store the jclass
                                                {                  // Methods to preload
                                                 {"toString", "()Ljava/lang/String;", &_method_toString}}};

const SimpleJNI::AutoRegister<UUID> UUID::registrar{&descriptor};

UUID::UUID() : _obj() {}

UUID::UUID(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

std::string UUID::toString() {
    if (!_obj) throw std::runtime_error("UUID is not initialized");
    return _obj.call_string_method(_method_toString);
}

}  // namespace Android
}  // namespace SimpleBLE