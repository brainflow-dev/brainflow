#pragma once

#include "types/java/util/UUID.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace SimpleBLE {
namespace Android {

class ParcelUUID {
  public:
    ParcelUUID();
    ParcelUUID(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    UUID getUuid();

    jobject get() const { return _obj.get(); }  // TODO: Remove once nothing uses this

  private:
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_getUuid;

    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<ParcelUUID> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE
