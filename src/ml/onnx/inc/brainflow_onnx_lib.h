#pragma once

#include "brainflow_model_params.h"
#include "shared_export.h"

#ifdef __cplusplus
extern "C"
{
#endif
    SHARED_EXPORT int CALLING_CONVENTION prepare (
        void *input_data, struct BrainFlowModelParams *params);
    SHARED_EXPORT int CALLING_CONVENTION predict (double *data, int data_len, double *output,
        int *output_len, struct BrainFlowModelParams *params);
    SHARED_EXPORT int CALLING_CONVENTION release (struct BrainFlowModelParams *params);
#ifdef __cplusplus
}
#endif
