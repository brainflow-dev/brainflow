#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_lda_classifier.h"


int ConcentrationLDAClassifier::prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationLDAClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err, "Classifier has already been prepared.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    double value = 0.0;
    // coefficients from focus_classifier.py, first 5 - avg band powers, last 5 stddevs
    const double coefficients[10] = {-0.55210632, 1.57052022, -8.27351333, 9.56457464, 15.2160856,
        -17.137741, 16.4732439, -34.28861863, 12.20167636, 15.26712795};
    double intercept = 0.17768186;
    // undocumented feature(not recommended): may work without stddev but with worse accuracy
    for (int i = 0; i < std::min (data_len, 10); i++)
    {
        value += coefficients[i] * data[i];
    }
    double concentration = 1.0 / (1.0 + exp (-1.0 * (intercept + value)));
    *output = concentration;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationLDAClassifier::release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
