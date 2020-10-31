#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_regression_classifier.h"
#include "regression_model.h"


int ConcentrationRegressionClassifier::prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err,
            "Incorrect arguments. Data len must be 10 and pointers should be non null.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    double value = 0.0;
    // undocumented feature(not recommended): may work without stddev but with worse accuracy
    for (int i = 0; i < std::min (data_len, 10); i++)
    {
        value += regression_coefficients[i] * data[i];
    }
    double concentration = 1.0 / (1.0 + exp (-1.0 * (regression_intercept + value)));
    *output = concentration;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
