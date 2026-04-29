#include "ArrayList.h"

namespace Java::Util {

template <template <typename> class RefType>
SimpleJNI::GlobalRef<jclass> ArrayList<RefType>::_cls;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_constructor = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_constructor_capacity = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_iterator = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_add = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_add_at_index = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_remove_object = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_remove_index = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_clear = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_size = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_get = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_set = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_isEmpty = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_ensureCapacity = nullptr;

template <template <typename> class RefType>
jmethodID ArrayList<RefType>::_method_trimToSize = nullptr;

template <template <typename> class RefType>
const SimpleJNI::JNIDescriptor ArrayList<RefType>::descriptor{
    "java/util/ArrayList",  // Java class name
    &_cls,                  // Where to store the jclass
    {                       // Methods to preload
     {"<init>", "()V", &_constructor},
     {"<init>", "(I)V", &_constructor_capacity},
     {"iterator", "()Ljava/util/Iterator;", &_method_iterator},
     {"add", "(Ljava/lang/Object;)Z", &_method_add},
     {"add", "(ILjava/lang/Object;)V", &_method_add_at_index},
     {"remove", "(Ljava/lang/Object;)Z", &_method_remove_object},
     {"remove", "(I)Ljava/lang/Object;", &_method_remove_index},
     {"clear", "()V", &_method_clear},
     {"size", "()I", &_method_size},
     {"get", "(I)Ljava/lang/Object;", &_method_get},
     {"set", "(ILjava/lang/Object;)Ljava/lang/Object;", &_method_set},
     {"isEmpty", "()Z", &_method_isEmpty},
     {"ensureCapacity", "(I)V", &_method_ensureCapacity},
     {"trimToSize", "()V", &_method_trimToSize}}};

template <template <typename> class RefType>
const SimpleJNI::AutoRegister<ArrayList<RefType>> ArrayList<RefType>::registrar{&descriptor};

template <template <typename> class RefType>
ArrayList<RefType>::ArrayList() : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("ArrayList JNI resources not preloaded");
    }

    SimpleJNI::Env env;
    jobject obj = env->NewObject(_cls.get(), _constructor);
    if (obj == nullptr) {
        throw std::runtime_error("Failed to create ArrayList instance");
    }

    _obj = SimpleJNI::Object<RefType, jobject>(obj, _cls.get());
    env->DeleteLocalRef(obj);
}

template <template <typename> class RefType>
ArrayList<RefType>::ArrayList(int initialCapacity) : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("ArrayList JNI resources not preloaded");
    }

    SimpleJNI::Env env;
    jobject obj = env->NewObject(_cls.get(), _constructor_capacity, initialCapacity);
    if (obj == nullptr) {
        throw std::runtime_error("Failed to create ArrayList instance with capacity");
    }

    _obj = SimpleJNI::Object<RefType, jobject>(obj, _cls.get());
    env->DeleteLocalRef(obj);
}

template <template <typename> class RefType>
ArrayList<RefType>::ArrayList(jobject obj) : _obj() {
    if (!_cls.get()) {
        throw std::runtime_error("ArrayList JNI resources not preloaded");
    }
    _obj = SimpleJNI::Object<RefType, jobject>(obj, _cls.get());
}

template <template <typename> class RefType>
template <template <typename> class OtherRefType>
ArrayList<RefType>::ArrayList(const SimpleJNI::Object<OtherRefType, jobject>& obj) : _obj(obj.get()) {}

template <template <typename> class RefType>
ArrayList<SimpleJNI::LocalRef> ArrayList<RefType>::to_local() const {
    if (!*this) return ArrayList<SimpleJNI::LocalRef>();
    return ArrayList<SimpleJNI::LocalRef>(_obj.get());
}

template <template <typename> class RefType>
ArrayList<SimpleJNI::GlobalRef> ArrayList<RefType>::to_global() const {
    if (!*this) return ArrayList<SimpleJNI::GlobalRef>();
    return ArrayList<SimpleJNI::GlobalRef>(_obj.get());
}

template <template <typename> class RefType>
jobject ArrayList<RefType>::get() const {
    return _obj.get();
}

template <template <typename> class RefType>
template <template <typename> class R>
typename std::enable_if<std::is_same<R<jobject>, SimpleJNI::ReleasableLocalRef<jobject>>::value, jobject>::type
ArrayList<RefType>::release() {
    return _obj.release();
}

template <template <typename> class RefType>
ArrayList<RefType>::operator bool() const {
    return _obj.get() != nullptr;
}

template <template <typename> class RefType>
ArrayList<RefType>::operator SimpleJNI::Object<RefType, jobject>() const {
    return _obj;
}

template <template <typename> class RefType>
Iterator<SimpleJNI::LocalRef> ArrayList<RefType>::iterator() const {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    jobject iter = _obj.call_object_method(_method_iterator).get();
    return Iterator<SimpleJNI::LocalRef>(iter);
}

template <template <typename> class RefType>
void ArrayList<RefType>::add(const SimpleJNI::Object<RefType, jobject>& element) {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    _obj.call_boolean_method(_method_add, element.get());
}

template <template <typename> class RefType>
void ArrayList<RefType>::add(size_t index, const SimpleJNI::Object<RefType, jobject>& element) {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    _obj.call_void_method(_method_add_at_index, static_cast<jint>(index), element.get());
}

template <template <typename> class RefType>
bool ArrayList<RefType>::remove(const SimpleJNI::Object<RefType, jobject>& element) {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    return _obj.call_boolean_method(_method_remove_object, element.get());
}

template <template <typename> class RefType>
SimpleJNI::Object<SimpleJNI::LocalRef> ArrayList<RefType>::remove(size_t index) {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    return _obj.call_object_method(_method_remove_index, static_cast<jint>(index));
}

template <template <typename> class RefType>
void ArrayList<RefType>::clear() {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    _obj.call_void_method(_method_clear);
}

template <template <typename> class RefType>
size_t ArrayList<RefType>::size() const {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    return static_cast<size_t>(_obj.call_int_method(_method_size));
}

template <template <typename> class RefType>
SimpleJNI::Object<SimpleJNI::LocalRef> ArrayList<RefType>::get(size_t index) const {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    return _obj.call_object_method(_method_get, static_cast<jint>(index));
}

template <template <typename> class RefType>
SimpleJNI::Object<SimpleJNI::LocalRef> ArrayList<RefType>::set(size_t index,
                                                               const SimpleJNI::Object<RefType, jobject>& element) {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    return _obj.call_object_method(_method_set, static_cast<jint>(index), element.get());
}

template <template <typename> class RefType>
bool ArrayList<RefType>::isEmpty() const {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    return _obj.call_boolean_method(_method_isEmpty);
}

template <template <typename> class RefType>
void ArrayList<RefType>::ensureCapacity(int minCapacity) {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    _obj.call_void_method(_method_ensureCapacity, minCapacity);
}

template <template <typename> class RefType>
void ArrayList<RefType>::trimToSize() {
    if (!*this) {
        throw std::runtime_error("ArrayList is not initialized");
    }
    _obj.call_void_method(_method_trimToSize);
}

// Explicit template instantiations
template class ArrayList<SimpleJNI::LocalRef>;
template class ArrayList<SimpleJNI::GlobalRef>;
template class ArrayList<SimpleJNI::WeakRef>;
template class ArrayList<SimpleJNI::ReleasableLocalRef>;

template jobject ArrayList<SimpleJNI::ReleasableLocalRef>::release<SimpleJNI::ReleasableLocalRef>();

}  // namespace Java::Util