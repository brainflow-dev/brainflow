#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "base_classifier.h"
#include "brainflow_constants.h"
#include "brainflow_model_params.h"
#include "brainflow_version.h"
#include "dyn_lib_classifier.h"
#include "mindfulness_classifier.h"
#include "ml_module.h"
#include "onnx_classifier.h"
#include "restfulness_classifier.h"

#include "json.hpp"

using json = nlohmann::json;

int string_to_brainflow_model_params (const char *json_params, struct BrainFlowModelParams *params);

std::map<struct BrainFlowModelParams, std::shared_ptr<BaseClassifier>> ml_models;
std::mutex models_mutex;


int prepare (const char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    std::shared_ptr<BaseClassifier> model = NULL;
    BaseClassifier::ml_logger->trace ("(Prepararing)Incoming json: {}", json_params);
    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::MINDFULNESS, (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER);
    int res = string_to_brainflow_model_params (json_params, &key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    if (ml_models.find (key) != ml_models.end ())
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }

    if ((key.metric == (int)BrainFlowMetrics::USER_DEFINED) &&
        (key.classifier == (int)BrainFlowClassifiers::DYN_LIB_CLASSIFIER))
    {
        model = std::shared_ptr<BaseClassifier> (new DynLibClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::USER_DEFINED) &&
        (key.classifier == (int)BrainFlowClassifiers::ONNX_CLASSIFIER))
    {
        model = std::shared_ptr<BaseClassifier> (new OnnxClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::MINDFULNESS) &&
        (key.classifier == (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER))
    {
        model = std::shared_ptr<BaseClassifier> (new MindfulnessClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::RESTFULNESS) &&
        (key.classifier == (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER))
    {
        model = std::shared_ptr<BaseClassifier> (new RestfulnessClassifier (key));
    }
    else
    {
        return (int)BrainFlowExitCodes::UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR;
    }

    res = model->prepare ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        BaseClassifier::ml_logger->error ("Unable to prepare model. Please refer to logs above.");
        model = NULL;
    }
    else
    {
        ml_models[key] = model;
    }
    return res;
}

int predict (double *data, int data_len, double *output, int *output_len, const char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);
    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::MINDFULNESS, (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER);
    BaseClassifier::ml_logger->trace ("(Predict)Incoming json: {}", json_params);
    int res = string_to_brainflow_model_params (json_params, &key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    auto model = ml_models.find (key);
    if (model == ml_models.end ())
    {
        BaseClassifier::ml_logger->error ("Must prepare model before using it for prediction.");
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    return model->second->predict (data, data_len, output, output_len);
}

int release (const char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::MINDFULNESS, (int)BrainFlowClassifiers::DEFAULT_CLASSIFIER);
    BaseClassifier::ml_logger->trace ("(Release)Incoming json: {}", json_params);
    int res = string_to_brainflow_model_params (json_params, &key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }

    auto model = ml_models.find (key);
    if (model == ml_models.end ())
    {
        BaseClassifier::ml_logger->error ("Must prepare model before releasing it.");
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    res = model->second->release ();
    ml_models.erase (model);
    return res;
}

int string_to_brainflow_model_params (const char *json_params, struct BrainFlowModelParams *params)
{
    // input string -> json -> struct BrainFlowModelParams
    try
    {
        json config = json::parse (std::string (json_params));
        params->metric = config["metric"];
        params->classifier = config["classifier"];
        params->file = config["file"];
        params->output_name = config["output_name"];
        params->other_info = config["other_info"];
        params->max_array_size = config["max_array_size"];
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    catch (json::exception &e)
    {
        BaseClassifier::ml_logger->error (
            "Unable to create Brainflow model params with these arguments. Exception: {}",
            e.what ());
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
}

int log_message_ml_module (int log_level, char *log_message)
{
    // its a method for loggging from high level api dont add it to Classifier class since it should
    // not be used internally
    std::lock_guard<std::mutex> lock (models_mutex);
    if (log_level < 0)
    {
        BaseClassifier::ml_logger->warn ("log level should be >= 0");
        log_level = 0;
    }
    else if (log_level > 6)
    {
        BaseClassifier::ml_logger->warn ("log level should be <= 6");
        log_level = 6;
    }

    BaseClassifier::ml_logger->log (spdlog::level::level_enum (log_level), "{}", log_message);

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int set_log_level_ml_module (int log_level)
{
    std::lock_guard<std::mutex> lock (models_mutex);
    return BaseClassifier::set_log_level (log_level);
}

int set_log_file_ml_module (const char *log_file)
{
    std::lock_guard<std::mutex> lock (models_mutex);
    return BaseClassifier::set_log_file (log_file);
}

int release_all ()
{
    std::lock_guard<std::mutex> lock (models_mutex);

    for (auto it = ml_models.begin (), next_it = it; it != ml_models.end (); it = next_it)
    {
        ++next_it;
        it->second->release ();
        ml_models.erase (it);
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int get_version_ml_module (char *version, int *num_chars, int max_chars)
{
    strncpy (version, BRAINFLOW_VERSION_STRING, max_chars);
    *num_chars = std::min<int> (max_chars, (int)strlen (BRAINFLOW_VERSION_STRING));
    return (int)BrainFlowExitCodes::STATUS_OK;
}