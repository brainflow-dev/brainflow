package org.simpleble.android

enum class BluetoothAddressType(val value: Int) {
    PUBLIC(0),
    RANDOM(1),
    UNSPECIFIED(2);

    override fun toString(): String {
        return when (this) {
            PUBLIC -> "Public"
            RANDOM -> "Random"
            UNSPECIFIED -> "Unspecified"
        }
    }

    companion object {
        fun fromInt(value: Int): BluetoothAddressType {
            return values().find { it.value == value } ?: UNSPECIFIED
        }
    }
}