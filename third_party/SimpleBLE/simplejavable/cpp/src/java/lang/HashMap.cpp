#include "HashMap.h"

namespace Java::Util {

template <template <typename> class RefType>
SimpleJNI::GlobalRef<jclass> HashMap<RefType>::_cls;

template <template <typename> class RefType>
jmethodID HashMap<RefType>::_method_init = nullptr;

template <template <typename> class RefType>
jmethodID HashMap<RefType>::_method_put = nullptr;

template <template <typename> class RefType>
const SimpleJNI::JNIDescriptor HashMap<RefType>::descriptor{
    "java/util/HashMap",  // Java class name
    &_cls,                // Where to store the jclass
    {                     // Methods to preload
     {"<init>", "()V", &_method_init},
     {"put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;", &_method_put}}};

template <template <typename> class RefType>
const SimpleJNI::AutoRegister<HashMap<RefType>> HashMap<RefType>::registrar{&descriptor};

template <template <typename> class RefType>
HashMap<RefType>::HashMap() : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("HashMap JNI resources not preloaded");
    }

    JNIEnv* env = SimpleJNI::VM::env();
    jobject obj = env->NewObject(_cls.get(), _method_init);
    _obj = SimpleJNI::Object<RefType, jobject>(obj, _cls.get());
}

template <template <typename> class RefType>
HashMap<RefType>::HashMap(jobject obj) : _obj(obj) {}

template <template <typename> class RefType>
template <template <typename> class OtherRefType>
HashMap<RefType>::HashMap(const SimpleJNI::Object<OtherRefType, jobject>& obj) : _obj(obj.get()) {}

template <template <typename> class RefType>
HashMap<SimpleJNI::LocalRef> HashMap<RefType>::to_local() const {
    if (!*this) return HashMap<SimpleJNI::LocalRef>();
    return HashMap<SimpleJNI::LocalRef>(_obj.get());
}

template <template <typename> class RefType>
HashMap<SimpleJNI::GlobalRef> HashMap<RefType>::to_global() const {
    if (!*this) return HashMap<SimpleJNI::GlobalRef>();
    return HashMap<SimpleJNI::GlobalRef>(_obj.get());
}

template <template <typename> class RefType>
jobject HashMap<RefType>::get() const {
    return _obj.get();
}

template <template <typename> class RefType>
template <template <typename> class R>
typename std::enable_if<std::is_same<R<jobject>, SimpleJNI::ReleasableLocalRef<jobject>>::value, jobject>::type
HashMap<RefType>::release() {
    return _obj.release();
}

template <template <typename> class RefType>
HashMap<RefType>::operator bool() const {
    return _obj.get() != nullptr;
}

template <template <typename> class RefType>
template <template <typename> class KeyRefType, template <typename> class ValueRefType>
SimpleJNI::Object<SimpleJNI::LocalRef> HashMap<RefType>::put(const SimpleJNI::Object<KeyRefType, jobject>& key,
                                                             const SimpleJNI::Object<ValueRefType, jobject>& value) {
    return _obj.call_object_method(_method_put, key.get(), value.get());
}

template <template <typename> class RefType>
HashMap<RefType>::operator SimpleJNI::Object<RefType, jobject>() const {
    return _obj;
}

// Explicit template instantiations
template class HashMap<SimpleJNI::LocalRef>;
template class HashMap<SimpleJNI::GlobalRef>;
template class HashMap<SimpleJNI::WeakRef>;
template class HashMap<SimpleJNI::ReleasableLocalRef>;

template SimpleJNI::Object<SimpleJNI::LocalRef, jobject> HashMap<SimpleJNI::ReleasableLocalRef>::put<
    SimpleJNI::LocalRef, SimpleJNI::LocalRef>(const SimpleJNI::Object<SimpleJNI::LocalRef, jobject>& key,
                                              const SimpleJNI::Object<SimpleJNI::LocalRef, jobject>& value);

template jobject HashMap<SimpleJNI::ReleasableLocalRef>::release<SimpleJNI::ReleasableLocalRef>();

}  // namespace Java::Util