#include "Iterator.h"

namespace Java::Util {

template <template <typename> class RefType>
SimpleJNI::GlobalRef<jclass> Iterator<RefType>::_cls;

template <template <typename> class RefType>
jmethodID Iterator<RefType>::_method_has_next = nullptr;

template <template <typename> class RefType>
jmethodID Iterator<RefType>::_method_next = nullptr;

template <template <typename> class RefType>
const SimpleJNI::JNIDescriptor Iterator<RefType>::descriptor{"java/util/Iterator",  // Java interface name
                                                             &_cls,                 // Where to store the jclass
                                                             {                      // Methods to preload
                                                              {"hasNext", "()Z", &_method_has_next},
                                                              {"next", "()Ljava/lang/Object;", &_method_next}}};

template <template <typename> class RefType>
const SimpleJNI::AutoRegister<Iterator<RefType>> Iterator<RefType>::registrar{&descriptor};

template <template <typename> class RefType>
Iterator<RefType>::Iterator() : _obj() {}

template <template <typename> class RefType>
Iterator<RefType>::Iterator(jobject obj) : _obj(obj) {
    if (!_cls.get()) {
        throw std::runtime_error("Iterator JNI resources not preloaded");
    }
    _obj = SimpleJNI::Object<RefType, jobject>(obj, _cls.get());
}

template <template <typename> class RefType>
template <template <typename> class OtherRefType>
Iterator<RefType>::Iterator(const SimpleJNI::Object<OtherRefType, jobject>& obj) : _obj(obj.get()) {}

template <template <typename> class RefType>
Iterator<SimpleJNI::LocalRef> Iterator<RefType>::to_local() const {
    if (!*this) return Iterator<SimpleJNI::LocalRef>();
    return Iterator<SimpleJNI::LocalRef>(_obj.get());
}

template <template <typename> class RefType>
Iterator<SimpleJNI::GlobalRef> Iterator<RefType>::to_global() const {
    if (!*this) return Iterator<SimpleJNI::GlobalRef>();
    return Iterator<SimpleJNI::GlobalRef>(_obj.get());
}

template <template <typename> class RefType>
jobject Iterator<RefType>::get() const {
    return _obj.get();
}

template <template <typename> class RefType>
Iterator<RefType>::operator bool() const {
    return _obj.get() != nullptr;
}

template <template <typename> class RefType>
bool Iterator<RefType>::has_next() const {
    if (!*this) return false;
    return _obj.call_boolean_method(_method_has_next);
}

template <template <typename> class RefType>
SimpleJNI::Object<SimpleJNI::LocalRef> Iterator<RefType>::next() {
    if (!*this) {
        throw std::runtime_error("Iterator is not initialized");
    }
    return _obj.call_object_method(_method_next);
}

template <template <typename> class RefType>
Iterator<RefType>::operator SimpleJNI::Object<RefType, jobject>() const {
    return _obj;
}

// Explicit template instantiations
template class Iterator<SimpleJNI::LocalRef>;
template class Iterator<SimpleJNI::GlobalRef>;
template class Iterator<SimpleJNI::WeakRef>;
template class Iterator<SimpleJNI::ReleasableLocalRef>;
}  // namespace Java::Util