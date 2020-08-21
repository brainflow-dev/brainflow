#pragma once

#include "brainflow_model_params.h"


class BaseClassifier
{
protected:
    struct BrainFlowModelParams params;

public:
    BaseClassifier (struct BrainFlowModelParams model_params) : params (model_params)
    {
    }

    virtual ~BaseClassifier ()
    {
    }

    virtual int prepare () = 0;
    virtual int predict (double *data, int data_len, double *output) = 0;
    virtual int release () = 0;
};