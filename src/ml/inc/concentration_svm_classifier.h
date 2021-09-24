#pragma once

#include "base_classifier.h"
#include "svm.h"

class ConcentrationSVMClassifier : public BaseClassifier
{
public:
    ConcentrationSVMClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        model = NULL;
    }

    virtual ~ConcentrationSVMClassifier ()
    {
        skip_logs = true;
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();

private:
    struct svm_model *model;
};
