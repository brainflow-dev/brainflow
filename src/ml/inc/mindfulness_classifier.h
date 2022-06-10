#pragma once

#include "base_classifier.h"


class MindfulnessClassifier : public BaseClassifier
{
public:
    MindfulnessClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
    }

    virtual ~MindfulnessClassifier ()
    {
        skip_logs = true;
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output, int *output_len);
    virtual int release ();
};