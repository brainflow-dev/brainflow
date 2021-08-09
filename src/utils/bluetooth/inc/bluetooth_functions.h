#pragma once

#include "shared_export.h"

enum class SocketBluetoothReturnCodes : int
{
    STATUS_OK = 0,
    WSA_STARTUP_ERROR = 1,
    CREATE_SOCKET_ERROR = 2,
    CONNECT_ERROR = 3,
    WSA_ADDR_ERROR = 4,
    IOCTL_ERROR = 5,
    ANOTHER_DEVICE_IS_CREATED = 6,
    DEVICE_IS_NOT_CREATED = 7
};

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_open_device (int, char *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_get_data (char *, int, char *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_write_data (char *, int, char *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_close_device (char *);
#ifdef __cplusplus
}
#endif
