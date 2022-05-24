#include "spdlog/spdlog.h"

#include "brainflow_constants.h"
#include "brainflow_model_params.h"
#include "brainflow_onnx_worker.h"

void log_onnx_msg (void *param, OrtLoggingLevel severity, const char *category, const char *logid,
    const char *code_location, const char *message)
{
    BaseClassifier *classifier = (BaseClassifier *)param;
    if (classifier != NULL)
    {
        classifier->safe_logger (
            spdlog::level::trace, "msg from onnx: {}, code location: {}", message, code_location);
    }
}

int BrainFlowONNXWorker::prepare ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    struct BrainFlowModelParams params = classifier->params;
    if (params.file.empty ())
    {
        classifier->safe_logger (spdlog::level::err, "file with onnx model is not provided");
        res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        ort = OrtGetApiBase ()->GetApi (7);
        if (ort == NULL)
        {
            classifier->safe_logger (spdlog::level::err, "Ort GetApi failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtStatus *onnx_status = ort->CreateEnvWithCustomLogger (log_onnx_msg, (void *)classifier,
            ORT_LOGGING_LEVEL_VERBOSE, "brainflow_onnx_lib", &env);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            classifier->safe_logger (spdlog::level::err, "CreateEnv failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtStatus *onnx_status = ort->CreateSessionOptions (&session_options);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            classifier->safe_logger (spdlog::level::err, "CreateSessionOptions failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
#ifdef _WIN32
        wchar_t model_path[1024];
        mbstowcs (model_path, params.file.c_str (), 1024);
        OrtStatus *onnx_status = ort->CreateSession (env, model_path, session_options, &session);
#else
        OrtStatus *onnx_status =
            ort->CreateSession (env, params.file.c_str (), session_options, &session);
#endif
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            classifier->safe_logger (spdlog::level::err, "CreateSession failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = check_input_types ();
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        release ();
    }
    return res;
}


int BrainFlowONNXWorker::predict (double *data, int data_len, double *output, int *output_len)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainFlowONNXWorker::release ()
{
    if (ort == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    if (session_options != NULL)
    {
        ort->ReleaseSessionOptions (session_options);
        session_options = NULL;
    }
    if (session != NULL)
    {
        ort->ReleaseSession (session);
        session = NULL;
    }
    if (env != NULL)
    {
        ort->ReleaseEnv (env);
        env = NULL;
    }
    ort = NULL;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BrainFlowONNXWorker::check_input_types ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    size_t num_input_nodes = 0;
    size_t num_output_nodes = 0;
    // input count
    OrtStatus *onnx_status = ort->SessionGetInputCount (session, &num_input_nodes);
    if (onnx_status != NULL)
    {
        const char *msg = ort->GetErrorMessage (onnx_status);
        classifier->safe_logger (spdlog::level::err, "SessionGetInputCount failed: {}", msg);
        ort->ReleaseStatus (onnx_status);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (num_input_nodes != 1)
        {
            classifier->safe_logger (spdlog::level::err, "Only one input node is supported");
            res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    // output count
    onnx_status = ort->SessionGetOutputCount (session, &num_output_nodes);
    if (onnx_status != NULL)
    {
        const char *msg = ort->GetErrorMessage (onnx_status);
        classifier->safe_logger (spdlog::level::err, "SessionGetInputCount failed: {}", msg);
        ort->ReleaseStatus (onnx_status);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (num_output_nodes != 1)
        {
            classifier->safe_logger (spdlog::level::err, "Only one output node is supported");
            res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    // input node types
    OrtTypeInfo *typeinfo;
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->SessionGetInputTypeInfo (session, 0, &typeinfo);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            classifier->safe_logger (spdlog::level::err, "SessionGetInputTypeInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    const OrtTensorTypeAndShapeInfo *tensor_info;
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->CastTypeInfoToTensorInfo (typeinfo, &tensor_info);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            classifier->safe_logger (
                spdlog::level::err, "CastTypeInfoToTensorInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->GetTensorElementType (tensor_info, &input_type);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            classifier->safe_logger (spdlog::level::err, "GetTensorElementType failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            classifier->safe_logger (spdlog::level::info, "input type is: {}", (int)input_type);
        }
    }

    // check input dims
    size_t num_dims = 0;
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->GetDimensionsCount (tensor_info, &num_dims);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            classifier->safe_logger (spdlog::level::err, "GetDimensionsCount failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            classifier->safe_logger (spdlog::level::info, "num dims is: {}", num_dims);
        }
    }

    input_node_dims.resize (num_dims);
    onnx_status = ort->GetDimensions (tensor_info, (int64_t *)input_node_dims.data (), num_dims);
    if (onnx_status != NULL)
    {
        const char *msg = ort->GetErrorMessage (onnx_status);
        classifier->safe_logger (spdlog::level::err, "GetDimensions failed: {}", msg);
        ort->ReleaseStatus (onnx_status);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else
    {
        for (size_t j = 0; j < num_dims; j++)
        {
            classifier->safe_logger (spdlog::level::info, "Dim {} size {}", j, input_node_dims[j]);
        }
    }
    ort->ReleaseTypeInfo (typeinfo);
    return res;
}