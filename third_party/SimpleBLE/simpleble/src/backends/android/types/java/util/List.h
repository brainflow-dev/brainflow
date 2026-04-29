#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"
#include "Iterator.h"
#include <vector>

namespace SimpleBLE {
namespace Android {

class List {
  public:
    List(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    Iterator iterator();

  private:
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_iterator;

    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<List> registrar;

    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;
};
}  // namespace Android
}  // namespace SimpleBLE