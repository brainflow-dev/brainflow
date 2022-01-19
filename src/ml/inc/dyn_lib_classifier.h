#pragma once

#include "base_classifier.h"
#include "runtime_dll_loader.h"


class DynLibClassifier : public BaseClassifier
{
public:
    DynLibClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        dll_loader = NULL;
    }

    virtual ~DynLibClassifier ()
    {
        skip_logs = true;
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();

protected:
    DLLLoader *dll_loader;
};
