package org.simplejavable;

public class BluetoothUUID {
    private final String uuid;

    public BluetoothUUID(String uuid) {
        this.uuid = uuid;
    }

    @Override
    public String toString() {
        return uuid;
    }
}