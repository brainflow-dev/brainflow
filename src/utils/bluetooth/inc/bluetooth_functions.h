#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_open_device (int, char *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_get_data (char *, int, char *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_write_data (char *, int, char *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_close_device (char *);
    SHARED_EXPORT int CALLING_CONVENTION bluetooth_discover_device (char *, char *, int *);
#ifdef __cplusplus
}
#endif
