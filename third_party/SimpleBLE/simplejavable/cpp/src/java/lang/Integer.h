#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Java::Util {

template <template <typename> class RefType>
class Integer {
  public:
    Integer();
    explicit Integer(jobject obj);
    explicit Integer(int value);

    template <template <typename> class OtherRefType>
    Integer(const SimpleJNI::Object<OtherRefType, jobject>& obj);

    // Conversion methods
    Integer<SimpleJNI::LocalRef> to_local() const;
    Integer<SimpleJNI::GlobalRef> to_global() const;

    // Get the underlying jobject
    jobject get() const;

    // Check if the object is valid
    explicit operator bool() const;

    // Implicit conversion to SimpleJNI::Object
    operator SimpleJNI::Object<RefType, jobject>() const;

    // Integer methods
    int int_value() const;

  private:
    // Underlying JNI object
    SimpleJNI::Object<RefType, jobject> _obj;

    // Static JNI resources
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_init;
    static jmethodID _method_int_value;
    static jmethodID _method_value_of;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<Integer> registrar;
};

}  // namespace Java::Util
