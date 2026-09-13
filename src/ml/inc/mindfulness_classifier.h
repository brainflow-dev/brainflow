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

protected:
    int calculate (double *data, int data_len, double *output, int *output_len) override;
};
