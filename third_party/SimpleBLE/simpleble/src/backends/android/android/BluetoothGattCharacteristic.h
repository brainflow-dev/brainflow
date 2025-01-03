#pragma once

#include <string>
#include <vector>
#include "BluetoothGattDescriptor.h"
#include "jni/Common.hpp"

namespace SimpleBLE {
namespace Android {

class BluetoothGattCharacteristic {
  // See: https://developer.android.com/reference/android/bluetooth/BluetoothGattCharacteristic
  public:
    BluetoothGattCharacteristic();
    BluetoothGattCharacteristic(JNI::Object obj);

    //    bool addDescriptor(BluetoothGattDescriptor descriptor);
    //    BluetoothGattDescriptor getDescriptor(std::string uuid);
    std::vector<BluetoothGattDescriptor> getDescriptors();

    int getInstanceId();
    int getPermissions();
    int getProperties();
    std::string getUuid();
    int getWriteType();
    void setWriteType(int writeType);

    bool setValue(const std::vector<uint8_t>& value);

    JNI::Object getObject() const { return _obj; }

    static const int PROPERTY_INDICATE = 0x00000020;
    static const int PROPERTY_NOTIFY = 0x00000010;
    static const int PROPERTY_READ = 0x00000002;
    static const int PROPERTY_WRITE = 0x00000008;
    static const int PROPERTY_WRITE_NO_RESPONSE = 0x00000004;

    static const int WRITE_TYPE_DEFAULT = 2;
    static const int WRITE_TYPE_NO_RESPONSE = 1;

  private:
    JNI::Object _obj;
    static JNI::Class _cls;
    static jmethodID _method_addDescriptor;
    static jmethodID _method_getDescriptor;
    static jmethodID _method_getDescriptors;
    static jmethodID _method_getInstanceId;
    static jmethodID _method_getPermissions;
    static jmethodID _method_getProperties;
    static jmethodID _method_getService;
    static jmethodID _method_getUuid;
    static jmethodID _method_getWriteType;
    static jmethodID _method_setWriteType;
    static jmethodID _method_setValue;

    static void initialize();
    void check_initialized() const;
};

}  // namespace Android
}  // namespace SimpleBLE
