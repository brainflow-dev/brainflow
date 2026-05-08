package org.simplejavable;

public class BluetoothAddress {
    private final String address;

    public BluetoothAddress(String address) {
        this.address = address;
    }

    @Override
    public String toString() {
        return address;
    }
}