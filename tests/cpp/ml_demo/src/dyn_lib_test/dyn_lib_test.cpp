#include <algorithm>
#include <cmath>

#include "brainflow_constants.h"
#include "dyn_lib_test.h"


int prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int predict (double *data, int data_len, double *output)
{
    const double regression_coefficients[10] = {
        -3.340, -3.40, -30.87, 17.87, 34.23, -1.087, -0.074, -1.843, 1.86, -1.366};
    double regression_intercept = 4.49;

    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double value = 0.0;
    for (int i = 0; i < std::min (data_len, 10); i++)
    {
        value += regression_coefficients[i] * data[i];
    }
    double concentration = 1.0 / (1.0 + exp (-1.0 * (regression_intercept + value)));
    *output = concentration;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}