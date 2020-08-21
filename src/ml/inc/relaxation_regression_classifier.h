#pragma once

#include "brainflow_constants.h"
#include "concentration_regression_classifier.h"


class RelaxationRegressionClassifier : public ConcentrationRegressionClassifier
{
public:
    RelaxationRegressionClassifier (struct BrainFlowModelParams params)
        : ConcentrationRegressionClassifier (params)
    {
    }

    int predict (double *data, int data_len, double *output)
    {
        int res = ConcentrationRegressionClassifier::predict (data, data_len, output);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        *output = 1.0 - (*output);
        return res;
    }
};
