#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Java::Util {

template <template <typename> class RefType>
class HashMap {
  public:
    HashMap();
    explicit HashMap(jobject obj);

    template <template <typename> class OtherRefType>
    HashMap(const SimpleJNI::Object<OtherRefType, jobject>& obj);

    // Conversion methods
    HashMap<SimpleJNI::LocalRef> to_local() const;
    HashMap<SimpleJNI::GlobalRef> to_global() const;

    // Get the underlying jobject
    jobject get() const;

    // Release the underlying jobject
    template <template <typename> class R = RefType>
    typename std::enable_if<std::is_same<R<jobject>, SimpleJNI::ReleasableLocalRef<jobject>>::value, jobject>::type
    release();

    // Check if the object is valid
    explicit operator bool() const;

    // Implicit conversion to SimpleJNI::Object
    operator SimpleJNI::Object<RefType, jobject>() const;

    // Put a key-value pair into the map
    template <template <typename> class KeyRefType, template <typename> class ValueRefType>
    SimpleJNI::Object<SimpleJNI::LocalRef> put(const SimpleJNI::Object<KeyRefType, jobject>& key,
                                               const SimpleJNI::Object<ValueRefType, jobject>& value);

  private:
    // Underlying JNI object
    SimpleJNI::Object<RefType, jobject> _obj;

    // Static JNI resources
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_init;
    static jmethodID _method_put;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<HashMap> registrar;
};

}  // namespace Java::Util