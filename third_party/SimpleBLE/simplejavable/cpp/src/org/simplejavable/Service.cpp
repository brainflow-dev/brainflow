#include "Service.h"

namespace Org {
namespace SimpleJavaBLE {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> Service::_cls;
jmethodID Service::_init_method;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor Service::descriptor{
    "org/simplejavable/Service",  // Java class name
    &_cls,                        // Where to store the jclass
    {
        {"<init>", "(Ljava/lang/String;Ljava/util/List;)V", &_init_method}  // Constructor method
    }};

// Define the AutoRegister instance
const SimpleJNI::AutoRegister<Service> Service::registrar{&descriptor};

// Constructor implementation
Service::Service(const SimpleJNI::String<SimpleJNI::LocalRef>& uuid,
                 const Java::Util::ArrayList<SimpleJNI::LocalRef>& characteristics) {
    // Use call_new_object to create a new Java Service object
    _obj = SimpleJNI::Object<SimpleJNI::ReleasableLocalRef>::call_new_object(_cls.get(), _init_method, uuid.get(),
                                                                             characteristics.get());
}

// Implement the conversion operator as a member function
Service::operator SimpleJNI::Object<SimpleJNI::ReleasableLocalRef, jobject>() const { return _obj; }

}  // namespace SimpleJavaBLE
}  // namespace Org