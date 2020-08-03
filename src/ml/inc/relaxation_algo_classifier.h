#pragma once

#include "brainflow_constants.h"
#include "concentration_algo_classifier.h"


class RelaxationAlgoClassifier : public ConcentrationAlgoClassifier
{
public:
    int predict (double *data, int data_len, double *output)
    {
        int res = ConcentrationAlgoClassifier::predict (data, data_len, output);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        *output = 1.0 - (*output);
        return res;
    }
};
