#include "List.h"
#include "Iterator.h"

namespace SimpleBLE {
namespace Android {

SimpleJNI::GlobalRef<jclass> List::_cls;
jmethodID List::_method_iterator = nullptr;

const SimpleJNI::JNIDescriptor List::descriptor{
    "java/util/List", // Java class name
    &_cls,            // Where to store the jclass
    {                 // Methods to preload
     {"iterator", "()Ljava/util/Iterator;", &_method_iterator}
    }
};

const SimpleJNI::AutoRegister<List> List::registrar{&descriptor};

List::List(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

Iterator List::iterator() {
    if (!_obj) throw std::runtime_error("List is not initialized");
    SimpleJNI::Object<SimpleJNI::LocalRef> iterator_obj = _obj.call_object_method(_method_iterator);
    return Iterator(iterator_obj.to_global());
}

}  // namespace Android
}  // namespace SimpleBLE