#include "brainflow_onnx_lib.h"
#include "brainflow_constants.h"


int prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int predict (double *data, int data_len, double *output)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}