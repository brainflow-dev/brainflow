#include "Characteristic.h"

namespace Org {
namespace SimpleJavaBLE {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> Characteristic::_cls;
jmethodID Characteristic::_init_method;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor Characteristic::descriptor{
    "org/simplejavable/Characteristic",  // Java class name
    &_cls,                               // Where to store the jclass
    {
        {"<init>", "(Ljava/lang/String;Ljava/util/List;ZZZZZ)V", &_init_method}  // Constructor method
    }};

// Define the AutoRegister instance
const SimpleJNI::AutoRegister<Characteristic> Characteristic::registrar{&descriptor};

// Constructor implementation
Characteristic::Characteristic(const SimpleJNI::String<SimpleJNI::LocalRef>& uuid,
                               const Java::Util::ArrayList<SimpleJNI::LocalRef>& descriptors, bool canRead,
                               bool canWriteRequest, bool canWriteCommand, bool canNotify, bool canIndicate) {
    // Use call_new_object to create a new Java Characteristic object
    _obj = SimpleJNI::Object<SimpleJNI::ReleasableLocalRef>::call_new_object(
        _cls.get(), _init_method, uuid.get(), descriptors.get(), static_cast<jboolean>(canRead),
        static_cast<jboolean>(canWriteRequest), static_cast<jboolean>(canWriteCommand),
        static_cast<jboolean>(canNotify), static_cast<jboolean>(canIndicate));
}

Characteristic::operator SimpleJNI::Object<SimpleJNI::ReleasableLocalRef, jobject>() const { return _obj; }

}  // namespace SimpleJavaBLE
}  // namespace Org