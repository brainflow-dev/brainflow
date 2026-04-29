#pragma once

#include <cstddef>
#include "Iterator.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace Java::Util {

template <template <typename> class RefType>
class ArrayList {
  public:
    ArrayList();
    explicit ArrayList(jobject obj);
    ArrayList(int initialCapacity);

    template <template <typename> class OtherRefType>
    ArrayList(const SimpleJNI::Object<OtherRefType, jobject>& obj);

    // Conversion methods
    ArrayList<SimpleJNI::LocalRef> to_local() const;
    ArrayList<SimpleJNI::GlobalRef> to_global() const;

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

    // ArrayList methods
    Iterator<SimpleJNI::LocalRef> iterator() const;
    void add(const SimpleJNI::Object<RefType, jobject>& element);
    void add(size_t index, const SimpleJNI::Object<RefType, jobject>& element);
    bool remove(const SimpleJNI::Object<RefType, jobject>& element);
    SimpleJNI::Object<SimpleJNI::LocalRef> remove(size_t index);
    void clear();
    size_t size() const;
    SimpleJNI::Object<SimpleJNI::LocalRef> get(size_t index) const;
    SimpleJNI::Object<SimpleJNI::LocalRef> set(size_t index, const SimpleJNI::Object<RefType, jobject>& element);
    bool isEmpty() const;
    void ensureCapacity(int minCapacity);
    void trimToSize();

  private:
    // Underlying JNI object
    SimpleJNI::Object<RefType, jobject> _obj;

    // Static JNI resources
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _constructor;
    static jmethodID _constructor_capacity;
    static jmethodID _method_iterator;
    static jmethodID _method_add;
    static jmethodID _method_add_at_index;
    static jmethodID _method_remove_object;
    static jmethodID _method_remove_index;
    static jmethodID _method_clear;
    static jmethodID _method_size;
    static jmethodID _method_get;
    static jmethodID _method_set;
    static jmethodID _method_isEmpty;
    static jmethodID _method_ensureCapacity;
    static jmethodID _method_trimToSize;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<ArrayList> registrar;
};

}  // namespace Java::Util