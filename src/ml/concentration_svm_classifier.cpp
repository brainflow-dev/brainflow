#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_svm_classifier.h"
#include "get_dll_dir.h"


int ConcentrationSVMClassifier::prepare ()
{
#ifdef __ANDROID__
    return (int)BrainFlowExitCodes::UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR;
#else
    char path[1024];
    bool res = get_dll_path (path);
    if (!res)
    {
        safe_logger (spdlog::level::err, "failed to determine dyn lib path.");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    char *full_path = (char *)malloc (std::strlen (path) + std::strlen ("brainflow_svm.model") + 1);
    std::strcpy (full_path, path);
    std::strcat (full_path, "brainflow_svm.model");
    model = svm_load_model (full_path);
    if (model == NULL)
    {
        safe_logger (spdlog::level::err, "failed to load model.");
        free (full_path);
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    if (svm_check_probability_model (model) == 0)
    {
        safe_logger (spdlog::level::err, "Model does not support probabiliy estimates.");
        free (full_path);
        svm_free_and_destroy_model (&model);
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    free (full_path);
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}

int ConcentrationSVMClassifier::predict (double *data, int data_len, double *output)
{
#ifdef __ANDROID__
    return (int)BrainFlowExitCodes::UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR;
#else
    if (model == NULL)
    {
        safe_logger (spdlog::level::err, "Please prepare classifier with prepare method.");
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    if ((data_len != 10) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err,
            "Incorrect arguments. Data len must be 10 and pointers should be non null.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    struct svm_node *x = (struct svm_node *)malloc ((data_len + 1) * sizeof (struct svm_node));
    for (int i = 0; i < data_len; i++)
    {
        x[i].index = i + 1;
        x[i].value = data[i];
    }
    x[data_len].index = -1;
    double prob_estimates[2];
    int labels[2];
    svm_get_labels (model, labels);
    svm_predict_probability (model, x, prob_estimates);
    *output = prob_estimates[(labels[1] & 1)];
    free (x);
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}

int ConcentrationSVMClassifier::release ()
{
#ifdef __ANDROID__
    return (int)BrainFlowExitCodes::UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR;
#else
    if (model == NULL)
    {
        safe_logger (spdlog::level::err, "Must prepare model before releasing it.");
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    svm_free_and_destroy_model (&model);
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}
