#include "SparseArray.h"

namespace SimpleBLE {
namespace Android {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> SparseArrayBase::_cls;
jmethodID SparseArrayBase::_method_size = nullptr;
jmethodID SparseArrayBase::_method_keyAt = nullptr;
jmethodID SparseArrayBase::_method_valueAt = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor SparseArrayBase::descriptor{
    "android/util/SparseArray", // Java class name
    &_cls,                      // Where to store the jclass
    {                           // Methods to preload
     {"size", "()I", &_method_size},
     {"keyAt", "(I)I", &_method_keyAt},
     {"valueAt", "(I)Ljava/lang/Object;", &_method_valueAt}
    }};

const SimpleJNI::AutoRegister<SparseArrayBase> SparseArrayBase::registrar{&descriptor};

template <typename T>
SparseArray<T>::SparseArray() {}

template <typename T>
SparseArray<T>::SparseArray(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}


template <typename T>
int SparseArray<T>::size() {
    if (!_obj) throw std::runtime_error("SparseArray is not initialized");
    return _obj.call_int_method(_method_size);
}

template <typename T>
int SparseArray<T>::keyAt(int index) {
    if (!_obj) throw std::runtime_error("SparseArray is not initialized");
    return _obj.call_int_method(_method_keyAt, index);
}

template <typename T>
T SparseArray<T>::valueAt(int index) {
    if (!_obj) throw std::runtime_error("SparseArray is not initialized");
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> value = _obj.call_object_method(_method_valueAt, index);
    return T(value.get());
}

template class SparseArray<SimpleJNI::ByteArray<SimpleJNI::LocalRef>>;
template class SparseArray<SimpleJNI::Object<SimpleJNI::LocalRef, jobject>>;

}  // namespace Android
}  // namespace SimpleBLE