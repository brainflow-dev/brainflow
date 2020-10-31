#pragma once

#include "brainflow_constants.h"
#include "concentration_svm_classifier.h"


class RelaxationSVMClassifier : public ConcentrationSVMClassifier
{
public:
    RelaxationSVMClassifier (struct BrainFlowModelParams params)
        : ConcentrationSVMClassifier (params)
    {
    }

    int predict (double *data, int data_len, double *output)
    {
        int res = ConcentrationSVMClassifier::predict (data, data_len, output);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        *output = 1.0 - (*output);
        return res;
    }
};
