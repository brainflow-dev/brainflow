#include "base_classifier.h"
#include "onnxruntime_c_api.h"


class BrainFlowONNXWorker
{
private:
    const OrtApi *ort;
    OrtEnv *env;
    OrtSessionOptions *session_options;
    OrtSession *session;
    ONNXTensorElementDataType input_type;
    std::vector<int64_t> input_node_dims;

    int check_input_types ();

public:
    BaseClassifier *classifier;

    BrainFlowONNXWorker (BaseClassifier *classifier)
    {
        this->classifier = classifier;
        ort = NULL;
        env = NULL;
        session_options = NULL;
        session = NULL;
        input_type = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
    }

    ~BrainFlowONNXWorker ()
    {
        classifier = NULL;
        release ();
    }

    int prepare ();
    int predict (double *data, int data_len, double *output, int *output_len);
    int release ();
};
