#pragma once

#include <simpleble/Types.h>
#include <string>
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace SimpleBLE {
namespace Android {

class BluetoothGattDescriptor {
    // See: https://developer.android.com/reference/android/bluetooth/BluetoothGattDescriptor
  public:
    BluetoothGattDescriptor();
    BluetoothGattDescriptor(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);
    virtual ~BluetoothGattDescriptor() = default;

    std::string getUuid();

    std::vector<uint8_t> getValue();
    bool setValue(const std::vector<uint8_t>& value);

    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> getObject() const { return _obj; }

    static const std::string CLIENT_CHARACTERISTIC_CONFIG;
    static const std::vector<uint8_t> DISABLE_NOTIFICATION_VALUE;
    static const std::vector<uint8_t> ENABLE_NOTIFICATION_VALUE;
    static const std::vector<uint8_t> ENABLE_INDICATION_VALUE;

  private:
    // Underlying JNI object - Use SimpleJNI::Object with GlobalRef
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_getUuid;
    static jmethodID _method_getValue;
    static jmethodID _method_setValue;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<BluetoothGattDescriptor> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE
