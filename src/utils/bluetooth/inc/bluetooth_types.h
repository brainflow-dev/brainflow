#pragma once


enum class SocketBluetoothReturnCodes : int
{
    // OS_SPECIFIC_ERROR is not used directly, but may be returned by OS specific socket libraries

    STATUS_OK = 0,
    OS_SPECIFIC_ERROR = -1,
    CONNECT_ERROR = -2,
    DISCONNECT_ERROR = -3,
    ANOTHER_DEVICE_IS_CREATED_ERROR = -4,
    DEVICE_IS_NOT_CREATED_ERROR = -5,
    DEVICE_IS_NOT_DISCOVERABLE = -6,
    PARAMETER_ERROR = -7,
    UNIMPLEMENTED_ERROR = -8
};
