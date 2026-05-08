#pragma once

#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

#include "types/java/util/UUID.h"
#include <map>
#include <vector>
#include "kvn/kvn_bytearray.h"
#include "types/android/os/ParcelUUID.h"
#include "types/android/util/SparseArray.h"

namespace SimpleBLE {
namespace Android {

class ScanRecord {
  public:
    ScanRecord(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    std::vector<std::string> getServiceUuids();
    std::map<uint16_t, kvn::bytearray> getManufacturerData();

    std::string toString();

  private:
    // Underlying JNI object
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_getServiceUuids;
    static jmethodID _method_getManufacturerData;
    static jmethodID _method_toString;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<ScanRecord> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE