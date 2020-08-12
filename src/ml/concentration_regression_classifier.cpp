#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_regression_classifier.h"


int ConcentrationRegressionClassifier::prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    double value = 0.0;
    // coefficients from focus_classifier.py
    const double coefficients[10] = {-13.8830376, -0.97060348, -4.12432519, 10.16475248, 8.85737357,
        -4.52375507, 0.98010763, -2.62295152, 0.70503625, -0.17565674};
    double intercept = 12.76772048;
    // undocumented feature: may work without stddev but with worse accuracy
    for (int i = 0; i < std::min (data_len, 10); i++)
    {
        value += coefficients[i] * data[i];
    }
    double concentration = 1.0 / (1.0 + exp (-1.0 * (intercept + value)));
    *output = concentration;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
