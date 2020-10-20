#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include "brainflow_constants.h"
#include "concentration_SVM_classifier.h"


int ConcentrationSVMClassifier::prepare ()
{
    model = svm_load_model ("brainflow_svm.model");
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationSVMClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err, "Classifier has already been prepared.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    struct svm_node *x;
    x = (struct svm_node *) realloc (x,data_len*sizeof(struct svm_node));
    for (int i = 0; i < data_len;i++)
    {
        x[i].index = i;
        x[i].value = data[i];
    }
    double *prob_estimates = NULL;
    double concentration = svm_predict_probability (model, x, prob_estimates);
    *output = concentration;
    delete [] x;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationSVMClassifier::release ()
{
    delete model;
    model = NULL;
    return (int)BrainFlowExitCodes::STATUS_OK;
}
