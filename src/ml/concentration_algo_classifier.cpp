#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_algo_classifier.h"


int ConcentrationAlgoClassifier::prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationAlgoClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    // beta / (alpha + theta) as here
    // https://www.sciencedirect.com/science/article/abs/pii/0301051195051163
    double concentration = data[3] / (data[2] + data[1]);
    // apply sigmoid function to scale to [0,1)
    concentration = concentration / (1.0 + abs (concentration));
    *output = concentration;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationAlgoClassifier::release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
