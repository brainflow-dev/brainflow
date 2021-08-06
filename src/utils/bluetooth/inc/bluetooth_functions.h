#pragma once

#include "bluetooth_types.h"
#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_get_data (BLUETOOTH_HANDLE, char *, int);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_open_device (
        int port, char *mac_address, BLUETOOTH_HANDLE *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_close_device (BLUETOOTH_HANDLE *);
#ifdef __cplusplus
}
#endif
