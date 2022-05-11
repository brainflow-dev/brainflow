#include "dyn_lib_classifier.h"
#include "brainflow_constants.h"


int DynLibClassifier::prepare ()
{
    if (dll_loader != NULL)
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }
    if (params.file.empty ())
    {
        safe_logger (spdlog::level::err, "dyn lib path is not provided.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    dll_loader = new DLLLoader (params.file.c_str ());
    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library");
        delete dll_loader;
        dll_loader = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    int (*func) () = (int (*) ())dll_loader->get_address ("prepare");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for prepare");
        delete dll_loader;
        dll_loader = NULL;
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return func ();
}

int DynLibClassifier::predict (double *data, int data_len, double *output)
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    int (*func) (double *, int, double *) =
        (int (*) (double *, int, double *))dll_loader->get_address ("predict");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for predict");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return func (data, data_len, output);
}

int DynLibClassifier::release ()
{
    if (dll_loader == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    int (*func) () = (int (*) ())dll_loader->get_address ("release");
    if (func == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for release");
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return func ();
}
