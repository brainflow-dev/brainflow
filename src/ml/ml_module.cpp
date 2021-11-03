#include <map>
#include <memory>
#include <mutex>
#include <utility>

#include "base_classifier.h"
#include "brainflow_constants.h"
#include "brainflow_model_params.h"
#include "concentration_knn_classifier.h"
#include "concentration_lda_classifier.h"
#include "concentration_regression_classifier.h"
#include "concentration_svm_classifier.h"
#include "dyn_lib_classifier.h"
#include "ml_module.h"
#include "relaxation_knn_classifier.h"
#include "relaxation_lda_classifier.h"
#include "relaxation_regression_classifier.h"
#include "relaxation_svm_classifier.h"

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
        (int)BrainFlowMetrics::CONCENTRATION, (int)BrainFlowClassifiers::REGRESSION);
    int res = string_to_brainflow_model_params (json_params, &key);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    if (ml_models.find (key) != ml_models.end ())
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }

    if ((key.metric == (int)BrainFlowMetrics::RELAXATION) &&
        (key.classifier == (int)BrainFlowClassifiers::REGRESSION))
    {
        model = std::shared_ptr<BaseClassifier> (new RelaxationRegressionClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::CONCENTRATION) &&
        (key.classifier == (int)BrainFlowClassifiers::REGRESSION))
    {
        model = std::shared_ptr<BaseClassifier> (new ConcentrationRegressionClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::RELAXATION) &&
        (key.classifier == (int)BrainFlowClassifiers::SVM))
    {
        model = std::shared_ptr<BaseClassifier> (new RelaxationSVMClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::CONCENTRATION) &&
        (key.classifier == (int)BrainFlowClassifiers::SVM))
    {
        model = std::shared_ptr<BaseClassifier> (new ConcentrationSVMClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::RELAXATION) &&
        (key.classifier == (int)BrainFlowClassifiers::LDA))
    {
        model = std::shared_ptr<BaseClassifier> (new RelaxationLDAClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::CONCENTRATION) &&
        (key.classifier == (int)BrainFlowClassifiers::LDA))
    {
        model = std::shared_ptr<BaseClassifier> (new ConcentrationLDAClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::CONCENTRATION) &&
        (key.classifier == (int)BrainFlowClassifiers::KNN))
    {
        model = std::shared_ptr<BaseClassifier> (new ConcentrationKNNClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::RELAXATION) &&
        (key.classifier == (int)BrainFlowClassifiers::KNN))
    {
        model = std::shared_ptr<BaseClassifier> (new RelaxationKNNClassifier (key));
    }
    else if ((key.metric == (int)BrainFlowMetrics::USER_DEFINED) &&
        (key.classifier == (int)BrainFlowClassifiers::DYN_LIB_CLASSIFIER))
    {
        model = std::shared_ptr<BaseClassifier> (new DynLibClassifier (key));
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

int predict (double *data, int data_len, double *output, const char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);
    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::CONCENTRATION, (int)BrainFlowClassifiers::REGRESSION);
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
    return model->second->predict (data, data_len, output);
}

int release (const char *json_params)
{
    std::lock_guard<std::mutex> lock (models_mutex);

    struct BrainFlowModelParams key (
        (int)BrainFlowMetrics::CONCENTRATION, (int)BrainFlowClassifiers::REGRESSION);
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
        params->other_info = config["other_info"];
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
