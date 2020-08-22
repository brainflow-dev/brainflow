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
    // coefficients from focus_classifier.py, first 5 - avg band powers, last 5 stddevs
    const double coefficients[10] = {-4.55141431, 0.64367822, -9.61285589, 4.96647132, 12.52338374,
        -4.40990224, 4.09532588, -7.01725697, 2.75863581, 0.24754745};
    double intercept = 3.96926308;
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
