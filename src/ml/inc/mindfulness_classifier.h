#pragma once

#include "base_classifier.h"


class MindfulnessClassifier : public BaseClassifier
{
public:
    MindfulnessClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
    }

    ~MindfulnessClassifier () override
    {
        skip_logs = true;
        release ();
    }

    int prepare () override;
    int predict (double *data, int data_len, double *output, int *output_len) override;
    int release () override;
};
