#pragma once

#include "BluetoothGattService.h"
#include "simplejni/Common.hpp"
#include "simplejni/Registry.hpp"

#include <cstdint>
#include <vector>

namespace SimpleBLE {
namespace Android {

class BluetoothGatt {
  public:

    static constexpr int CONNECTION_PRIORITY_BALANCED = 0;
    static constexpr int CONNECTION_PRIORITY_HIGH = 1;
    static constexpr int CONNECTION_PRIORITY_LOW_POWER = 2;
    static constexpr int CONNECTION_PRIORITY_DCK = 3;

    BluetoothGatt();
    BluetoothGatt(SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> obj);

    void close();
    bool connect();
    void disconnect();
    bool discoverServices();
    std::vector<BluetoothGattService> getServices();

    // void abortReliableWrite(BluetoothDevice mDevice);
    // void abortReliableWrite();
    // bool beginReliableWrite();
    // bool executeReliableWrite();
    // std::list<BluetoothDevice> getConnectedDevices();
    // int getConnectionState(BluetoothDevice device);
    // BluetoothDevice getDevice();
    // std::list<BluetoothDevice> getDevicesMatchingConnectionStates(std::vector<int>& states);
    // BluetoothGattService getService(UUID uuid);
    // std::list<BluetoothGattService> getServices();
    bool readCharacteristic(BluetoothGattCharacteristic characteristic);
    bool readDescriptor(BluetoothGattDescriptor descriptor);
    // void readPhy();
    // bool readRemoteRssi();
    bool requestConnectionPriority(int connectionPriority);
    // bool requestMtu(int mtu);
    bool setCharacteristicNotification(BluetoothGattCharacteristic characteristic, bool enable);
    // void setPreferredPhy(int txPhy, int rxPhy, int phyOptions);
    bool writeCharacteristic(BluetoothGattCharacteristic characteristic);
    // int writeCharacteristic(BluetoothGattCharacteristic characteristic, std::vector<byte>& value, int writeType);
    bool writeDescriptor(BluetoothGattDescriptor descriptor);
    // int writeDescriptor(BluetoothGattDescriptor descriptor, std::vector<byte>& value);

  private:
    SimpleJNI::Object<SimpleJNI::GlobalRef, jobject> _obj;

    // Static JNI resources managed by Registrar
    static SimpleJNI::GlobalRef<jclass> _cls;
    static jmethodID _method_close;
    static jmethodID _method_connect;
    static jmethodID _method_disconnect;
    static jmethodID _method_discoverServices;
    static jmethodID _method_getServices;
    static jmethodID _method_readCharacteristic;
    static jmethodID _method_readDescriptor;
    static jmethodID _method_setCharacteristicNotification;
    static jmethodID _method_writeCharacteristic;
    static jmethodID _method_writeDescriptor;
    static jmethodID _method_requestConnectionPriority;
    // JNI descriptor for auto-registration
    static const SimpleJNI::JNIDescriptor descriptor;
    static const SimpleJNI::AutoRegister<BluetoothGatt> registrar;
};

}  // namespace Android
}  // namespace SimpleBLE
