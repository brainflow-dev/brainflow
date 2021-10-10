#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION prepare (const char *json_params);
    SHARED_EXPORT int CALLING_CONVENTION predict (
        double *data, int data_len, double *output, const char *json_params);
    SHARED_EXPORT int CALLING_CONVENTION release (const char *json_params);

    // logging methods
    SHARED_EXPORT int CALLING_CONVENTION set_log_level_ml_module (int log_level);
    SHARED_EXPORT int CALLING_CONVENTION set_log_file_ml_module (const char *log_file);
#ifdef __cplusplus
}
#endif
