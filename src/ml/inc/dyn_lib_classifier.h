#pragma once

#include <string>

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
    virtual int predict (double *data, int data_len, double *output, int *output_len);
    virtual int release ();

protected:
    virtual std::string get_dyn_lib_path ()
    {
        return params.file;
    }

    DLLLoader *dll_loader;
};
