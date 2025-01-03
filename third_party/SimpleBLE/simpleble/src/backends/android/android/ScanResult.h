#pragma once

#include "jni/Common.hpp"

#include "BluetoothDevice.h"

namespace SimpleBLE {
namespace Android {

class ScanResult {
  public:
    ScanResult(jobject j_scan_result);

    BluetoothDevice getDevice();
    std::string toString();

  private:
    static JNI::Class _cls;
    static void initialize();

    JNI::Object _obj;
};

}  // namespace Android
}  // namespace SimpleBLE
