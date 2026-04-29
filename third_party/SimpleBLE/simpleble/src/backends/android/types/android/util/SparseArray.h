#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace SimpleBLE {
namespace Android {

class SparseArrayBase {
  public:
    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_size;
    static jmethodID _method_keyAt;
    static jmethodID _method_valueAt;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<SparseArrayBase> registrar;
};

template <typename T>
class SparseArray : public SparseArrayBase {
  public:
    SparseArray();
    SparseArray(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    int size();
    int keyAt(int index);
    T valueAt(int index);

  private:
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;
};

}  // namespace Android
}  // namespace SimpleBLE
