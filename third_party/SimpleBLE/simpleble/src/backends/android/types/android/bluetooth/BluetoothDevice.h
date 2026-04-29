#pragma once

#include "BluetoothGatt.h"
#include "bridge/BluetoothGattCallback.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

namespace SimpleBLE {
namespace Android {

class BluetoothDevice {
  public:
    BluetoothDevice(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);
    virtual ~BluetoothDevice() = default;

    std::string getAddress();
    // int getAddressType();
    std::string getName();
    int getBondState();
    // void removeBond();

    BluetoothGatt connectGatt(bool autoConnect, Bridge::BluetoothGattCallback& callback);

    static const int BOND_NONE = 10;
    static const int BOND_BONDING = 11;
    static const int BOND_BONDED = 12;

    static const int ADDRESS_TYPE_ANONYMOUS = 255;
    static const int ADDRESS_TYPE_PUBLIC = 0;
    static const int ADDRESS_TYPE_RANDOM = 1;

    static const int TRANSPORT_AUTO = 0;
    static const int TRANSPORT_BREDR = 1;
    static const int TRANSPORT_LE = 2;

  private:
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_getAddress;
    // static jmethodID _method_getAddressType;
    static jmethodID _method_getName;
    static jmethodID _method_getBondState;
    static jmethodID _method_removeBond;
    static jmethodID _method_connectGatt;

    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<BluetoothDevice> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE
