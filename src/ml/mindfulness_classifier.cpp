#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "mindfulness_classifier.h"
#include "mindfulness_model.h"


int MindfulnessClassifier::calculate (double *data, int data_len, double *output, int *output_len)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL) || (output_len == NULL) ||
        (params.max_array_size < 1))
    {
        safe_logger (spdlog::level::err,
            "Incorrect arguments. Null pointers or invalid feature vector size.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    *output_len = 0;
    double value = mindfulness_intercept;
    // Preserve the historical contract: additional features are ignored.
    for (int i = 0; i < 5; i++)
    {
        if (!std::isfinite (data[i]))
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
        value += mindfulness_coefficients[i] * data[i];
    }
    if (!std::isfinite (value))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    const double exponential = std::exp (-std::abs (value));
    double mindfulness =
        (value >= 0.0) ? 1.0 / (1.0 + exponential) : exponential / (1.0 + exponential);
    *output = mindfulness;
    *output_len = 1;
    return (int)BrainFlowExitCodes::STATUS_OK;
}
