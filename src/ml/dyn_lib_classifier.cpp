#include "dyn_lib_classifier.h"
#include "brainflow_constants.h"


int DynLibClassifier::prepare ()
{
    if (dll_loader != NULL)
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }
    if (get_dyn_lib_path ().empty ())
    {
        safe_logger (spdlog::level::err, "dyn lib path is not provided.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    dll_loader = new DLLLoader (get_dyn_lib_path ().c_str ());
    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        delete dll_loader;
        dll_loader = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int (*func) (void *, struct BrainFlowModelParams *) =
        (int (*) (void *, struct BrainFlowModelParams *))dll_loader->get_address ("prepare");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for prepare");
        delete dll_loader;
        dll_loader = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return func ((void *)this, &params);
}

int DynLibClassifier::predict (double *data, int data_len, double *output, int *output_len)
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    int (*func) (double *, int, double *, int *, struct BrainFlowModelParams *) =
        (int (*) (double *, int, double *, int *,
            struct BrainFlowModelParams *))dll_loader->get_address ("predict");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for predict");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return func (data, data_len, output, output_len, &params);
}

int DynLibClassifier::release ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    int (*func) (struct BrainFlowModelParams *) =
        (int (*) (struct BrainFlowModelParams *))dll_loader->get_address ("release");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for release");
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        res = func (&params);
    }

    dll_loader->free_library ();
    delete dll_loader;
    dll_loader = NULL;

    return res;
}
