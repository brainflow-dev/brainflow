#include "ScanResult.h"

namespace SimpleBLE {
namespace Android {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> ScanResult::_cls;
jmethodID ScanResult::_method_getDevice = nullptr;
jmethodID ScanResult::_method_getRssi = nullptr;
jmethodID ScanResult::_method_getTxPower = nullptr;
jmethodID ScanResult::_method_isConnectable = nullptr;
jmethodID ScanResult::_method_getScanRecord = nullptr;
jmethodID ScanResult::_method_toString = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor ScanResult::descriptor{
    "android/bluetooth/le/ScanResult", // Java class name
    &_cls,                             // Where to store the jclass
    {                                  // Methods to preload
     {"getDevice", "()Landroid/bluetooth/BluetoothDevice;", &_method_getDevice},
     {"getRssi", "()I", &_method_getRssi},
     {"getTxPower", "()I", &_method_getTxPower},
     {"isConnectable", "()Z", &_method_isConnectable},
     {"getScanRecord", "()Landroid/bluetooth/le/ScanRecord;", &_method_getScanRecord},
     {"toString", "()Ljava/lang/String;", &_method_toString}
    }};

const SimpleJNI::AutoRegister<ScanResult> ScanResult::registrar{&descriptor};

ScanResult::ScanResult(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

BluetoothDevice ScanResult::getDevice() {
    if (!_obj) throw std::runtime_error("ScanResult object is not initialized");
    return BluetoothDevice(_obj.call_object_method(_method_getDevice));
}

int16_t ScanResult::getRssi() {
    if (!_obj) throw std::runtime_error("ScanResult object is not initialized");
    return _obj.call_int_method(_method_getRssi);
}

int16_t ScanResult::getTxPower() {
    if (!_obj) throw std::runtime_error("ScanResult object is not initialized");
    return _obj.call_int_method(_method_getTxPower);
}

bool ScanResult::isConnectable() {
    if (!_obj) throw std::runtime_error("ScanResult object is not initialized");
    return _obj.call_boolean_method(_method_isConnectable);
}

ScanRecord ScanResult::getScanRecord() {
    if (!_obj) throw std::runtime_error("ScanResult object is not initialized");
    return ScanRecord(_obj.call_object_method(_method_getScanRecord));
}

std::string ScanResult::toString() {
    if (!_obj) throw std::runtime_error("ScanResult object is not initialized");
    return _obj.call_string_method(_method_toString);
}

}  // namespace Android
}  // namespace SimpleBLE