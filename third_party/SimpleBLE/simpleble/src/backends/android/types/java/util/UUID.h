#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace SimpleBLE {
namespace Android {

class UUID {
  public:
    UUID();
    UUID(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    std::string toString();

    jobject get() const { return _obj.get(); }  // TODO: Remove once nothing uses this

  private:
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_toString;

    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<UUID> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE
