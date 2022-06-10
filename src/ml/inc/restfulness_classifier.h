#pragma once

#include "brainflow_constants.h"
#include "mindfulness_classifier.h"


class RestfulnessClassifier : public MindfulnessClassifier
{
public:
    RestfulnessClassifier (struct BrainFlowModelParams params) : MindfulnessClassifier (params)
    {
    }

    int predict (double *data, int data_len, double *output, int *output_len)
    {
        int res = MindfulnessClassifier::predict (data, data_len, output, output_len);
        if (res != (int)BrainFlowExitCodes::STATUS_OK)
        {
            return res;
        }
        *output = 1.0 - (*output);
        return res;
    }
};