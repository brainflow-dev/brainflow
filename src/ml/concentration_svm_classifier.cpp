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
    char *full_path = new char[std::strlen (path) + std::strlen ("brainflow_svm.model") + 1];
    std::strcpy (full_path, path);
    std::strcat (full_path, "brainflow_svm.model");
    model = svm_load_model (full_path);
    delete[] full_path;
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
        safe_logger (spdlog::level::err, "Model needs to be prepared before using it.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err,
            "Incorrect arguments. Data len must be >=5 and pointers should be non null.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    struct svm_node *x;
    x = (struct svm_node *)malloc ((data_len + 1) * sizeof (struct svm_node));
    for (int i = 0; i < data_len; i++)
    {
        x[i].index = i + 1;
        x[i].value = data[i];
    }
    x[data_len].index = -1;
    int nr_class = svm_get_nr_class (model);
    double *prob_estimates = (double *)malloc (nr_class * sizeof (double));
    svm_predict_probability (model, x, prob_estimates);
    *output = prob_estimates[0];
    free (x);
    free (prob_estimates);
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
        safe_logger (spdlog::level::err, "Model needs to be prepared before releasing it.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    svm_free_model_content (model);
    free (model);
    model = NULL;
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}
