#pragma once

#include "base_classifier.h"


class ConcentrationLDAClassifier : public BaseClassifier
{
public:
    ConcentrationLDAClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
    }

    virtual ~ConcentrationLDAClassifier ()
    {
        skip_logs = true;
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();
};
