#pragma once

#include <stdint.h>
#include <string>
#include <vector>

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
    OrtAllocator *allocator;

    ONNXTensorElementDataType input_type;
    std::vector<int64_t> input_node_dims;
    std::vector<const char *> input_node_names;

    ONNXTensorElementDataType output_type;
    std::vector<int64_t> output_node_dims;
    std::vector<const char *> output_node_names;

    DLLLoader *dll_loader;

    int load_api ();
    int get_input_info ();
    int get_output_info ();
    std::string get_onnxlib_path ();


public:
    OnnxClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        ort = NULL;
        env = NULL;
        session_options = NULL;
        session = NULL;
        allocator = NULL;
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
