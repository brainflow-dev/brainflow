#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION prepare (int metric, int classifier);
    SHARED_EXPORT int CALLING_CONVENTION predict (
        double *data, int data_len, double *output, int metric, int classifier);
    SHARED_EXPORT int CALLING_CONVENTION release (int metric, int classifier);
#ifdef __cplusplus
}
#endif
