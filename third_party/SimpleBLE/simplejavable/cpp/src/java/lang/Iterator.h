#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Java::Util {

template <template <typename> class RefType>
class Iterator {
  public:
    Iterator();
    explicit Iterator(jobject obj);

    template <template <typename> class OtherRefType>
    Iterator(const SimpleJNI::Object<OtherRefType, jobject>& obj);

    // Conversion methods
    Iterator<SimpleJNI::LocalRef> to_local() const;
    Iterator<SimpleJNI::GlobalRef> to_global() const;

    // Get the underlying jobject
    jobject get() const;

    // Check if the object is valid
    explicit operator bool() const;

    // Implicit conversion to SimpleJNI::Object
    operator SimpleJNI::Object<RefType, jobject>() const;

    // Iterator methods
    bool has_next() const;
    SimpleJNI::Object<SimpleJNI::LocalRef> next();

  private:
    // Underlying JNI object
    SimpleJNI::Object<RefType, jobject> _obj;

    // Static JNI resources
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_has_next;
    static jmethodID _method_next;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<Iterator> registrar;
};

}  // namespace Java::Util