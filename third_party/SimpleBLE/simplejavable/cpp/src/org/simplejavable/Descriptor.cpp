#include "Descriptor.h"

namespace Org {
namespace SimpleJavaBLE {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> Descriptor::_cls;
jmethodID Descriptor::_init_method;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor Descriptor::descriptor{
    "org/simplejavable/Descriptor",  // Java class name
    &_cls,                           // Where to store the jclass
    {
        {"<init>", "(Ljava/lang/String;)V", &_init_method}  // Constructor method
    }};

// Define the AutoRegister instance
const SimpleJNI::AutoRegister<Descriptor> Descriptor::registrar{&descriptor};

// Constructor implementation
Descriptor::Descriptor(const SimpleJNI::String<SimpleJNI::LocalRef>& uuid) {
    // Use call_new_object to create a new Java Descriptor object
    _obj = SimpleJNI::Object<SimpleJNI::ReleasableLocalRef>::call_new_object(_cls.get(), _init_method, uuid.get());
}

Descriptor::operator SimpleJNI::Object<SimpleJNI::ReleasableLocalRef, jobject>() const { return _obj; }

}  // namespace SimpleJavaBLE
}  // namespace Org