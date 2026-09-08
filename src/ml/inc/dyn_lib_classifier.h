#pragma once

#include <string>

#include "base_classifier.h"
#include "runtime_dll_loader.h"


// DynLibClassifier loads user-provided shared libraries (.so / .dll / .dylib) exporting
// "prepare", "predict", and "release" C functions.
// Note for plugin authors: BaseClassifier inspects params.other_info for moving average
// configuration via JSON. Recognized keys: "moving_average" (bool or int) and "window_len" (int).
// Non-JSON strings are safely ignored and will not activate smoothing.
class DynLibClassifier : public BaseClassifier
{
public:
    DynLibClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        dll_loader = NULL;
    }

    ~DynLibClassifier () override
    {
        skip_logs = true;
        release ();
    }

protected:
    int prepare_classifier () override;
    int calculate (double *data, int data_len, double *output, int *output_len) override;
    int release_classifier () override;

    virtual std::string get_dyn_lib_path ()
    {
        return params.file;
    }

    DLLLoader *dll_loader;
};
