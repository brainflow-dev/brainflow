#include "BluetoothScanner.h"
#include <CommonUtils.h>
#include <android/log.h>
#include <fmt/format.h>

namespace SimpleBLE {
namespace Android {

// Define static JNI resources
SimpleJNI::GlobalRef<jclass> BluetoothScanner::_cls;
jmethodID BluetoothScanner::_constructor = nullptr;
jmethodID BluetoothScanner::_method_startScan = nullptr;
jmethodID BluetoothScanner::_method_stopScan = nullptr;
jmethodID BluetoothScanner::_method_toString = nullptr;

// Define the JNI descriptor
const SimpleJNI::JNIDescriptor BluetoothScanner::descriptor{
    "android/bluetooth/le/BluetoothLeScanner", // Java class name
    &_cls,                                     // Where to store the jclass
    {                                          // Methods to preload
     {"<init>", "()V", &_constructor},
     {"startScan", "(Landroid/bluetooth/le/ScanCallback;)V", &_method_startScan},
     {"stopScan", "(Landroid/bluetooth/le/ScanCallback;)V", &_method_stopScan},
     {"toString", "()Ljava/lang/String;", &_method_toString}
    }};

const SimpleJNI::AutoRegister<BluetoothScanner> BluetoothScanner::registrar{&descriptor};

BluetoothScanner::BluetoothScanner(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj) : _obj(obj) {}

void BluetoothScanner::startScan(Bridge::ScanCallback& callback) {
    if (!_obj) throw std::runtime_error("BluetoothScanner is not initialized");
    _obj.call_void_method(_method_startScan, callback.get());
}

void BluetoothScanner::stopScan(Bridge::ScanCallback& callback) {
    if (!_obj) throw std::runtime_error("BluetoothScanner is not initialized");
    _obj.call_void_method(_method_stopScan, callback.get());
}

std::string BluetoothScanner::toString() {
    if (!_obj) throw std::runtime_error("BluetoothScanner is not initialized");
    return _obj.call_string_method(_method_toString);
}

}  // namespace Android
}  // namespace SimpleBLE