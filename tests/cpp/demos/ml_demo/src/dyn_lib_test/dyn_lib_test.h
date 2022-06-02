#pragma once

#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#define CALLING_CONVENTION __cdecl
#else
#define SHARED_EXPORT __attribute__ ((visibility ("default")))
#define CALLING_CONVENTION
#endif

#include "brainflow_model_params.h"


#ifdef __cplusplus
extern "C"
{
#endif

    SHARED_EXPORT int CALLING_CONVENTION prepare (
        void *custom, struct BrainFlowModelParams *params);
    SHARED_EXPORT int CALLING_CONVENTION predict (double *data, int data_len, double *output,
        int *output_len, struct BrainFlowModelParams *params);
    SHARED_EXPORT int CALLING_CONVENTION release (struct BrainFlowModelParams *params);

#ifdef __cplusplus
}
#endif
