#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"
#include <vector>

namespace SimpleBLE {
namespace Android {

class Iterator {
  public:
    Iterator(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    bool hasNext();
    SimpleJNI::Object<SimpleJNI::LocalRef> next();

  private:
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_hasNext;
    static jmethodID _method_next;

    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<Iterator> registrar;

    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;
};
}  // namespace Android
}  // namespace SimpleBLE