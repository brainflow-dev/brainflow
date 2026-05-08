package org.simplejavable;

public enum BluetoothAddressType {
    PUBLIC(0),
    RANDOM(1),
    UNSPECIFIED(2);

    private final int value;

    BluetoothAddressType(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    @Override
    public String toString() {
        switch (this) {
            case PUBLIC:
                return "Public";
            case RANDOM:
                return "Random";
            case UNSPECIFIED:
                return "Unspecified";
            default:
                return "Unspecified";
        }
    }

    public static BluetoothAddressType fromInt(int value) {
        for (BluetoothAddressType type : values()) {
            if (type.getValue() == value) {
                return type;
            }
        }
        return UNSPECIFIED;
    }
}