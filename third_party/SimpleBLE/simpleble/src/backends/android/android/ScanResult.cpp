
#include "ScanResult.h"

namespace SimpleBLE {
namespace Android {

JNI::Class ScanResult::_cls;

void ScanResult::initialize() {
    JNI::Env env;

    if (_cls.get() == nullptr) {
        _cls = env.find_class("android/bluetooth/le/ScanResult");
    }
}

ScanResult::ScanResult(jobject j_scan_result) {
    initialize();
    _obj = JNI::Object(j_scan_result, _cls.get());
};

BluetoothDevice ScanResult::getDevice() {
    return BluetoothDevice(_obj.call_object_method("getDevice", "()Landroid/bluetooth/BluetoothDevice;"));
}

std::string ScanResult::toString() { return _obj.call_string_method("toString", "()Ljava/lang/String;"); }

}  // namespace Android
}  // namespace SimpleBLE