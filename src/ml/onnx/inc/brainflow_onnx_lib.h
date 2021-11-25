#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION prepare ();
    SHARED_EXPORT int CALLING_CONVENTION predict (double *data, int data_len, double *output);
    SHARED_EXPORT int CALLING_CONVENTION release ();
#ifdef __cplusplus
}
#endif
