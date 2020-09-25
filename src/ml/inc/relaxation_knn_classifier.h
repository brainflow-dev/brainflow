#pragma once

#include "brainflow_constants.h"
#include "concentration_knn_classifier.h"


class RelaxationKNNClassifier : public ConcentrationKNNClassifier
{
public:
    RelaxationKNNClassifier (struct BrainFlowModelParams params)
        : ConcentrationKNNClassifier (params)
    {
    }

    int predict (double *data, int data_len, double *output)
    {
        int res = ConcentrationKNNClassifier::predict (data, data_len, output);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        *output = 1.0 - (*output);
        return res;
    }
};
