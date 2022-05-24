#include <map>
#include <mutex>
#include <utility>

#include "brainflow_constants.h"
#include "brainflow_onnx_lib.h"
#include "brainflow_onnx_worker.h"

std::map<struct BrainFlowModelParams, std::shared_ptr<BrainFlowONNXWorker>> classifiers;
std::mutex mutex;


int prepare (void *input_data, struct BrainFlowModelParams *params)
{
    std::lock_guard<std::mutex> lock (mutex);
    if (classifiers.find (*params) != classifiers.end ())
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }
    std::shared_ptr<BrainFlowONNXWorker> classifier = std::shared_ptr<BrainFlowONNXWorker> (
        new BrainFlowONNXWorker ((BaseClassifier *)input_data));
    int res = classifier->prepare ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        classifier = NULL;
    }
    else
    {
        classifiers[*params] = classifier;
    }
    return res;
}

int predict (double *data, int data_len, double *output, int *output_len,
    struct BrainFlowModelParams *params)
{
    std::lock_guard<std::mutex> lock (mutex);
    auto classifier_it = classifiers.find (*params);
    if (classifier_it == classifiers.end ())
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    return classifier_it->second->predict (data, data_len, output, output_len);
}

int release (struct BrainFlowModelParams *params)
{
    std::lock_guard<std::mutex> lock (mutex);
    auto classifier_it = classifiers.find (*params);
    if (classifier_it == classifiers.end ())
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    int res = classifier_it->second->release ();
    classifiers.erase (classifier_it);
    return res;
}