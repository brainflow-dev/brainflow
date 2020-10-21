#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <iostream>
#include "brainflow_constants.h"
#include "concentration_svm_classifier.h"
#include "get_dll_dir.h"

int ConcentrationSVMClassifier::prepare ()
{
    char path[1024];
    bool res = get_dll_path (path);
    char * fullPath = new char [std::strlen(path)+std::strlen("brainflow_svm.model")+1];
    std::strcpy (fullPath,path);
    std::strcat (fullPath ,"brainflow_svm.model");
    model = svm_load_model (fullPath);
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
    x =  (struct svm_node *) malloc(data_len*sizeof(struct svm_node));
    for (int i = 0; i < data_len;i++)
    {
        x[i].index = i;
        x[i].value = data[i];
    }
    int nr_class = svm_get_nr_class (model);
    double *prob_estimates = (double *) malloc(nr_class * sizeof(double));
    double concentration = svm_predict_probability (model, x, prob_estimates);
    for(int i = 0; i < nr_class;i++)
    {
        std::cout.precision(17);
        std::cout << "Prob estimates "<< i << " is " << prob_estimates[i] << "." << std::endl;
    }
    std::cout.precision (17);
    std::cout << "Concentration is " << prob_estimates[(int)concentration] << ".";
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
