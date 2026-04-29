#include "Integer.h"

namespace Java::Util {

template <template <typename> class RefType>
SimpleJNI::GlobalRef<jclass> Integer<RefType>::_cls;

template <template <typename> class RefType>
jmethodID Integer<RefType>::_method_init = nullptr;

template <template <typename> class RefType>
jmethodID Integer<RefType>::_method_int_value = nullptr;

template <template <typename> class RefType>
jmethodID Integer<RefType>::_method_value_of = nullptr;

template <template <typename> class RefType>
const SimpleJNI::JNIDescriptor Integer<RefType>::descriptor{"java/lang/Integer",  // Java class name
                                                            &_cls,                // Where to store the jclass
                                                            {
                                                                // Methods to preload
                                                                {"<init>", "(I)V", &_method_init},
                                                                {"intValue", "()I", &_method_int_value},
                                                            }};

template <template <typename> class RefType>
const SimpleJNI::AutoRegister<Integer<RefType>> Integer<RefType>::registrar{&descriptor};

template <template <typename> class RefType>
Integer<RefType>::Integer() : _obj() {}

template <template <typename> class RefType>
Integer<RefType>::Integer(jobject obj) : _obj(obj) {
    if (!_cls.get()) {
        throw std::runtime_error("Integer JNI resources not preloaded");
    }
}

template <template <typename> class RefType>
Integer<RefType>::Integer(int value) : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("Integer JNI resources not preloaded");
    }
    JNIEnv* env = SimpleJNI::VM::env();
    jobject obj = env->NewObject(_cls.get(), _method_init, value);
    _obj = SimpleJNI::Object<RefType, jobject>(obj, _cls.get());
}

template <template <typename> class RefType>
template <template <typename> class OtherRefType>
Integer<RefType>::Integer(const SimpleJNI::Object<OtherRefType, jobject>& obj) : _obj(obj.get()) {}

template <template <typename> class RefType>
Integer<SimpleJNI::LocalRef> Integer<RefType>::to_local() const {
    if (!*this) return Integer<SimpleJNI::LocalRef>();
    return Integer<SimpleJNI::LocalRef>(_obj.get());
}

template <template <typename> class RefType>
Integer<SimpleJNI::GlobalRef> Integer<RefType>::to_global() const {
    if (!*this) return Integer<SimpleJNI::GlobalRef>();
    return Integer<SimpleJNI::GlobalRef>(_obj.get());
}

template <template <typename> class RefType>
jobject Integer<RefType>::get() const {
    return _obj.get();
}

template <template <typename> class RefType>
Integer<RefType>::operator bool() const {
    return _obj.get() != nullptr;
}

template <template <typename> class RefType>
int Integer<RefType>::int_value() const {
    if (!*this) {
        throw std::runtime_error("Integer is not initialized");
    }
    return _obj.call_int_method(_method_int_value);
}

template <template <typename> class RefType>
Integer<RefType>::operator SimpleJNI::Object<RefType, jobject>() const {
    return _obj;
}

// Explicit template instantiations
template class Integer<SimpleJNI::LocalRef>;
template class Integer<SimpleJNI::GlobalRef>;
template class Integer<SimpleJNI::WeakRef>;
template class Integer<SimpleJNI::ReleasableLocalRef>;
}  // namespace Java::Util
