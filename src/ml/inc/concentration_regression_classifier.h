#pragma once

#include "base_classifier.h"


class ConcentrationRegressionClassifier : public BaseClassifier
{
public:
    ConcentrationRegressionClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
    }

    virtual ~ConcentrationRegressionClassifier ()
    {
        skip_logs = true;
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();
};
