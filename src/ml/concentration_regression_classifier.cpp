#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_regression_classifier.h"

#include "spdlog/sinks/null_sink.h"

int ConcentrationRegressionClassifier::prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err, "Classifier has already been prepared.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    double value = 0.0;
    // coefficients from focus_classifier.py, first 5 - avg band powers, last 5 stddevs
    const double coefficients[10] = {-4.00053368, -1.41176333, -15.76696787, 7.03090858,
        18.15572423, -3.69625477, 3.18110813, -8.27378987, 2.30053461, 2.94329616};
    double intercept = 4.00736793;
    // undocumented feature(not recommended): may work without stddev but with worse accuracy
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
