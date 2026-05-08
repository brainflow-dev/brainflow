#include "Set.h"
#include "Iterator.h" // Ensure Iterator.h is correctly included

namespace SimpleBLE {
namespace Android { // Changed namespace

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> Set::_cls;
jmethodID Set::_method_iterator = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor Set::descriptor{
    "java/util/Set",              // Java class name
    &_cls,                        // Where to store the jclass
    {                             // Methods to preload
     {"iterator", "()Ljava/util/Iterator;", &_method_iterator}
    }
};

const SimpleJNI::AutoRegister<Set> Set::registrar{&descriptor};

Set::Set(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

Iterator Set::iterator() {
    if (!_obj) throw std::runtime_error("Set is not initialized");
    SimpleJNI::Object<SimpleJNI::LocalRef> iterator_obj = _obj.call_object_method(_method_iterator);
    // Assuming Iterator's constructor now accepts SimpleJNI::Object<SimpleJNI::LocalRef>
    // or SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> as per previous refactoring.
    return Iterator(iterator_obj.to_global()); // Convert to GlobalRef if Iterator stores it as such
}

}  // namespace Android
}  // namespace SimpleBLE