#pragma once

#include <string>
#include <simpleble/Types.h>
#include "jni/Common.hpp"

namespace SimpleBLE {
namespace Android {

class BluetoothGattDescriptor {
  // See: https://developer.android.com/reference/android/bluetooth/BluetoothGattDescriptor
  public:
    BluetoothGattDescriptor();
    BluetoothGattDescriptor(JNI::Object obj);

    std::string getUuid();

    std::vector<uint8_t> getValue();
    bool setValue(const std::vector<uint8_t>& value);

    JNI::Object getObject() const { return _obj; }

    static const std::string CLIENT_CHARACTERISTIC_CONFIG;
    static const std::vector<uint8_t> DISABLE_NOTIFICATION_VALUE;
    static const std::vector<uint8_t> ENABLE_NOTIFICATION_VALUE;
    static const std::vector<uint8_t> ENABLE_INDICATION_VALUE;


  private:
    JNI::Object _obj;
    static JNI::Class _cls;
    static jmethodID _method_getUuid;
    static jmethodID _method_getValue;
    static jmethodID _method_setValue;

    static void initialize();
    void check_initialized() const;
};

}  // namespace Android
}  // namespace SimpleBLE
