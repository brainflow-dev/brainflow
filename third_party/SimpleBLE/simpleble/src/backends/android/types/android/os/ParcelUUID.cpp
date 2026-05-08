#include "ParcelUUID.h"

namespace SimpleBLE {
namespace Android {

// Static members definition
SimpleJNI::GlobalRef<jclass> ParcelUUID::_cls;
jmethodID ParcelUUID::_method_getUuid = nullptr;

// JNI Descriptor for ParcelUUID
const SimpleJNI::JNIDescriptor ParcelUUID::descriptor{
    "android/os/ParcelUuid",                             // Java class name
    &_cls,                                               // Pointer to store the jclass
    {                                                    // Methods to preload
     {"getUuid", "()Ljava/util/UUID;", &_method_getUuid}
    }};

// Auto-register the class with SimpleJNI
const SimpleJNI::AutoRegister<ParcelUUID> ParcelUUID::registrar{&descriptor};

ParcelUUID::ParcelUUID() : _obj() {}

ParcelUUID::ParcelUUID(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

UUID ParcelUUID::getUuid() {
    if (!_obj) throw std::runtime_error("ParcelUUID is not initialized");
    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> java_uuid_obj = _obj.call_object_method(_method_getUuid);
    return UUID(java_uuid_obj);
}

}  // namespace Android
}  // namespace SimpleBLE