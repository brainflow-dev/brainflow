package org.simpleble.android.bridge;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.util.Log;

public class BluetoothGattCallback extends android.bluetooth.BluetoothGattCallback {

    public BluetoothGattCallback() {}

    @Override
    public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        // NOTE: This method is deprecated on API 33, but older Android versions still call it.
        super.onCharacteristicChanged(gatt, characteristic);
        onCharacteristicChangedCallback(gatt, characteristic, characteristic.getValue());
    }

    public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value) {
        // API 33+ calls this overload directly; older android.jar versions do not declare it.
        onCharacteristicChangedCallback(gatt, characteristic, value);
    }

    @Override
    public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
        // NOTE: This method is deprecated on API 33, but older Android versions still call it.
        super.onCharacteristicRead(gatt, characteristic, status);
        onCharacteristicReadCallback(gatt, characteristic, characteristic.getValue(), status);
    }

    public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value, int status) {
        // API 33+ calls this overload directly; older android.jar versions do not declare it.
        onCharacteristicReadCallback(gatt, characteristic, value, status);
    }

    @Override
    public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
        super.onCharacteristicWrite(gatt, characteristic, status);
        onCharacteristicWriteCallback(gatt, characteristic, status);
    }

    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
        super.onConnectionStateChange(gatt, status, newState);
        onConnectionStateChangeCallback(gatt, status, newState);
    }

    @Override
    public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
        // NOTE: This method is deprecated on API 33, but older Android versions still call it.
        super.onDescriptorRead(gatt, descriptor, status);
        onDescriptorReadCallback(gatt, descriptor, descriptor.getValue(), status);
    }

    public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status, byte[] value) {
        // API 33+ calls this overload directly; older android.jar versions do not declare it.
        onDescriptorReadCallback(gatt, descriptor, value, status);
    }

    @Override
    public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
        super.onDescriptorWrite(gatt, descriptor, status);
        onDescriptorWriteCallback(gatt, descriptor, status);
    }

    @Override
    public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
        super.onMtuChanged(gatt, mtu, status);
        onMtuChangedCallback(gatt, mtu, status);
    }

    @Override
    public void onPhyRead(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
        super.onPhyRead(gatt, txPhy, rxPhy, status);
        onPhyReadCallback(gatt, txPhy, rxPhy, status);
    }

    @Override
    public void onPhyUpdate(BluetoothGatt gatt, int txPhy, int rxPhy, int status) {
        super.onPhyUpdate(gatt, txPhy, rxPhy, status);
        onPhyUpdateCallback(gatt, txPhy, rxPhy, status);
    }

    @Override
    public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
        super.onReadRemoteRssi(gatt, rssi, status);
        onReadRemoteRssiCallback(gatt, rssi, status);
    }

    @Override
    public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
        super.onReliableWriteCompleted(gatt, status);
        onReliableWriteCompletedCallback(gatt, status);
    }

    @Override
    public void onServiceChanged(BluetoothGatt gatt) {
        super.onServiceChanged(gatt);
        onServiceChangedCallback(gatt);
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
        super.onServicesDiscovered(gatt, status);
        onServicesDiscoveredCallback(gatt, status);
    }

    private native void onCharacteristicChangedCallback(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value);

    private native void onCharacteristicReadCallback(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, byte[] value, int status);

    private native void onCharacteristicWriteCallback(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status);

    private native void onConnectionStateChangeCallback(BluetoothGatt gatt, int status, int newState);

    private native void onDescriptorReadCallback(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, byte[] value, int status);

    private native void onDescriptorWriteCallback(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status);

    private native void onMtuChangedCallback(BluetoothGatt gatt, int mtu, int status);

    private native void onPhyReadCallback(BluetoothGatt gatt, int txPhy, int rxPhy, int status);

    private native void onPhyUpdateCallback(BluetoothGatt gatt, int txPhy, int rxPhy, int status);

    private native void onReadRemoteRssiCallback(BluetoothGatt gatt, int rssi, int status);

    private native void onReliableWriteCompletedCallback(BluetoothGatt gatt, int status);

    private native void onServiceChangedCallback(BluetoothGatt gatt);

    private native void onServicesDiscoveredCallback(BluetoothGatt gatt, int status);
}

