#pragma once

#include <string>
#include <vector>
#include "BluetoothGattCharacteristic.h"
#include "jni/Common.hpp"

namespace SimpleBLE {
namespace Android {

class BluetoothGattService {
  public:
    BluetoothGattService();
    BluetoothGattService(JNI::Object obj);

//    bool addCharacteristic(BluetoothGattCharacteristic characteristic);
//    bool addService(BluetoothGattService service);
//    BluetoothGattCharacteristic getCharacteristic(std::string uuid);
    std::vector<BluetoothGattCharacteristic> getCharacteristics();
//    std::vector<BluetoothGattService> getIncludedServices(); // TODO: This might be necessary if we don't see the secondary services in some other way.
    int getInstanceId();
    int getType();
    std::string getUuid();

    JNI::Object getObject() const { return _obj; }

  private:
    JNI::Object _obj;
    static JNI::Class _cls;
    static jmethodID _method_addCharacteristic;
    static jmethodID _method_addService;
    static jmethodID _method_getCharacteristic;
    static jmethodID _method_getCharacteristics;
    static jmethodID _method_getIncludedServices;
    static jmethodID _method_getInstanceId;
    static jmethodID _method_getType;
    static jmethodID _method_getUuid;

    static void initialize();
    void check_initialized() const;
};

}  // namespace Android
}  // namespace SimpleBLE
