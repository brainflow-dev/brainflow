#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "mindfulness_classifier.h"
#include "mindfulness_model.h"


int MindfulnessClassifier::prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MindfulnessClassifier::predict (double *data, int data_len, double *output, int *output_len)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err,
            "Incorrect arguments. Null pointers or invalid feature vector size.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    double value = 0.0;
    for (int i = 0; i < std::min (data_len, 5); i++)
    {
        value += mindfulness_coefficients[i] * data[i];
    }
    double mindfulness = 1.0 / (1.0 + exp (-1.0 * (mindfulness_intercept + value)));
    *output = mindfulness;
    *output_len = 1;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int MindfulnessClassifier::release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}