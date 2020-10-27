#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_lda_classifier.h"
#include "lda_model.h"

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

    // undocumented feature(not recommended): may work without stddev but with worse accuracy
    for (int i = 0; i < std::min (data_len, 10); i++)
    {
        value += lda_coefficients[i] * data[i];
    }
    double concentration = 1.0 / (1.0 + exp (-1.0 * (lda_intercept + value)));
    *output = concentration;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationLDAClassifier::release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
