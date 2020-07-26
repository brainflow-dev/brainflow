#pragma once

#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION prepare (int calculator);
    SHARED_EXPORT int CALLING_CONVENTION calc_concentration (double *avg_bandpowers,
        double *stddev_bandpowers, int data_len, double *output, int calculator);
    SHARED_EXPORT int CALLING_CONVENTION calc_relaxation (double *avg_bandpowers,
        double *stddev_bandpowers, int data_len, double *output, int calculator);
    SHARED_EXPORT int CALLING_CONVENTION release (int calculator);
#ifdef __cplusplus
}
#endif
