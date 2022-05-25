#pragma once

#include <string>

#include "base_classifier.h"
#include "dyn_lib_classifier.h"
#include "onnxruntime_c_api.h"


class OnnxClassifier : public BaseClassifier
{
private:
    const OrtApi *ort;
    OrtEnv *env;
    OrtSessionOptions *session_options;
    OrtSession *session;

    DLLLoader *dll_loader;

    int load_api ();
    std::string get_onnxlib_path ();

public:
    OnnxClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        ort = NULL;
        env = NULL;
        session_options = NULL;
        session = NULL;
        dll_loader = NULL;
    }

    ~OnnxClassifier ()
    {
        skip_logs = true;
        release ();
    }

    int prepare ();
    int predict (double *data, int data_len, double *output, int *output_len);
    int release ();
};
