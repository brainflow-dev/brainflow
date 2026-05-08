#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"
#include "Iterator.h"
#include <vector>

namespace SimpleBLE {
namespace Android {
class Set {
  public:
    Set(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    Iterator iterator();

  private:
    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_iterator;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<Set> registrar;

    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;
};
}  // namespace Android
}  // namespace SimpleBLE