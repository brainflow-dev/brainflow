#include "Iterator.h"

namespace SimpleBLE {
namespace Android {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> Iterator::_cls;
jmethodID Iterator::_method_hasNext = nullptr;
jmethodID Iterator::_method_next = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor Iterator::descriptor{
    "java/util/Iterator",             // Java class name
    &_cls,                            // Where to store the jclass
    {                                 // Methods to preload
     {"hasNext", "()Z", &_method_hasNext},
     {"next", "()Ljava/lang/Object;", &_method_next}
    }
};

const SimpleJNI::AutoRegister<Iterator> Iterator::registrar{&descriptor};

Iterator::Iterator(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

bool Iterator::hasNext() {
    if (!_obj) throw std::runtime_error("Iterator is not initialized");
    return _obj.call_boolean_method(_method_hasNext);
}

SimpleJNI::Object<SimpleJNI::LocalRef> Iterator::next() {
    if (!_obj) throw std::runtime_error("Iterator is not initialized");
    return _obj.call_object_method(_method_next);
}

}  // namespace Android
}  // namespace SimpleBLE