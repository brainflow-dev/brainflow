#include <algorithm>
#include <cmath>

#include "brainflow_constants.h"
#include "dyn_lib_test.h"


int prepare (void *custom, struct BrainFlowModelParams *params)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int predict (double *data, int data_len, double *output, int *output_len,
    struct BrainFlowModelParams *params)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // some semi random coeffs from log reg model
    const double regression_coefficients[5] = {-3.340, -3.40, -30.87, 17.87, 34.23};
    double regression_intercept = 4.49;
    double value = 0.0;
    for (int i = 0; i < 5; i++)
    {
        value += regression_coefficients[i] * data[i];
    }
    double concentration = 1.0 / (1.0 + exp (-1.0 * (regression_intercept + value)));
    output[0] = concentration;
    *output_len = 1;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int release (struct BrainFlowModelParams *params)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}