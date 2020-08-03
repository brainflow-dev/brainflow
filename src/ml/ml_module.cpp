#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "base_classifier.h"
#include "brainflow_constants.h"
#include "concentration_algo_classifier.h"
#include "ml_module.h"
#include "relaxation_algo_classifier.h"


std::map<std::pair<int, int>, std::shared_ptr<BaseClassifier>> ml_models;
std::mutex models_mutex;

std::pair<int, int> get_key (int metric, int classifier);
int check_model (int metric, int classifier, std::pair<int, int> &key);


int prepare (int metric, int classifier)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    std::shared_ptr<BaseClassifier> model = NULL;
    std::pair<int, int> key = get_key (metric, classifier);
    if (ml_models.find (key) != ml_models.end ())
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }

    if (key ==
        std::make_pair ((int)BrainFlowMetrics::RELAXATION, (int)BrainFlowClassifiers::ALGORITHMIC))
    {
        model = std::shared_ptr<BaseClassifier> (new RelaxationAlgoClassifier ());
    }
    else if (key ==
        std::make_pair (
            (int)BrainFlowMetrics::CONCENTRATION, (int)BrainFlowClassifiers::ALGORITHMIC))
    {
        model = std::shared_ptr<BaseClassifier> (new ConcentrationAlgoClassifier ());
    }
    else
    {
        return (int)BrainFlowExitCodes::UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR;
    }

    int res = model->prepare ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        model = NULL;
    }
    else
    {
        ml_models[key] = model;
    }
    return res;
}

int predict (double *data, int data_len, double *output, int metric, int classifier)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    std::pair<int, int> key;
    int res = check_model (metric, classifier, key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto model = ml_models.find (key);
    return model->second->predict (data, data_len, output);
}

int release (int metric, int classifier)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    std::pair<int, int> key;
    int res = check_model (metric, classifier, key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto model = ml_models.find (key);
    res = model->second->release ();
    ml_models.erase (model);
    return res;
}


/////////////////////////////////////////////////
//////////////////// helpers ////////////////////
/////////////////////////////////////////////////


std::pair<int, int> get_key (int metric, int classifier)
{
    std::pair<int, int> key = std::make_pair (metric, classifier);
    return key;
}

int check_model (int metric, int classifier, std::pair<int, int> &key)
{
    key = get_key (metric, classifier);

    if (ml_models.find (key) == ml_models.end ())
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
