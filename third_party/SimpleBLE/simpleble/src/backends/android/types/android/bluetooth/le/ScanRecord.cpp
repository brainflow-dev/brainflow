#include "ScanRecord.h"
#include "types/android/os/ParcelUUID.h"
#include "types/android/util/SparseArray.h"
#include "types/java/util/List.h"
#include "types/java/util/Iterator.h"
namespace SimpleBLE {
namespace Android {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> ScanRecord::_cls;
jmethodID ScanRecord::_method_getServiceUuids = nullptr;
jmethodID ScanRecord::_method_getManufacturerData = nullptr;
jmethodID ScanRecord::_method_toString = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor ScanRecord::descriptor{
    "android/bluetooth/le/ScanRecord", // Java class name
    &_cls,                             // Where to store the jclass
    {                                  // Methods to preload
     {"getServiceUuids", "()Ljava/util/List;", &_method_getServiceUuids},
     {"getManufacturerSpecificData", "()Landroid/util/SparseArray;", &_method_getManufacturerData},
     {"toString", "()Ljava/lang/String;", &_method_toString}
    }};

const SimpleJNI::AutoRegister<ScanRecord> ScanRecord::registrar{&descriptor};

ScanRecord::ScanRecord(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

std::vector<std::string> ScanRecord::getServiceUuids() {
    if (!_obj) throw std::runtime_error("ScanRecord object is not initialized");

    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> service_uuids_obj = _obj.call_object_method(_method_getServiceUuids);
    if (!service_uuids_obj) return {};

    std::vector<std::string> result;
    List list(service_uuids_obj.to_global());
    Iterator iterator = list.iterator();
    while (iterator.hasNext()) {
        ParcelUUID parcel_uuid = ParcelUUID(iterator.next());
        result.push_back(parcel_uuid.getUuid().toString());
    }

    return result;
}

std::map<uint16_t, kvn::bytearray> ScanRecord::getManufacturerData() {
    if (!_obj) throw std::runtime_error("ScanRecord object is not initialized");

    SimpleJNI::Object<SimpleJNI::LocalRef, jobject> manufacturer_data_obj = _obj.call_object_method(_method_getManufacturerData);
    if (!manufacturer_data_obj) return {};

    SparseArray<SimpleJNI::ByteArray<SimpleJNI::LocalRef>> sparse_array(manufacturer_data_obj);

    std::map<uint16_t, kvn::bytearray> result;
    for (int i = 0; i < sparse_array.size(); i++) {
        uint16_t key = sparse_array.keyAt(i);
        SimpleJNI::ByteArray<SimpleJNI::LocalRef> value = sparse_array.valueAt(i);
        result[key] = value.bytes();
    }
    return result;
}

std::string ScanRecord::toString() {
    if (!_obj) throw std::runtime_error("ScanRecord object is not initialized");
    return _obj.call_string_method(_method_toString);
}

}  // namespace Android
}  // namespace SimpleBLE