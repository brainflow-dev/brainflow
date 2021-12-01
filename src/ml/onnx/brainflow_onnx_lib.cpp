#include <cstdlib>
#include <string>

#include "brainflow_constants.h"
#include "brainflow_model_params.h"
#include "brainflow_onnx_lib.h"
#include "onnxruntime_c_api.h"

const OrtApi *g_ort = NULL;
OrtEnv *g_env = NULL;
OrtSessionOptions *g_session_options = NULL;
OrtSession *g_session = NULL;

int prepare (void *input_data)
{
    struct BrainFlowModelParams *params = (struct BrainFlowModelParams *)(input_data);
    if (g_ort != NULL)
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }

    if (params->file.empty ())
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    g_ort = OrtGetApiBase ()->GetApi (ORT_API_VERSION);
    if (g_ort == NULL)
    {
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtStatus *onnx_status =
            g_ort->CreateEnv (ORT_LOGGING_LEVEL_VERBOSE, "brainflow_onnx_lib", &g_env);
        if (onnx_status != NULL)
        {
            g_ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtStatus *onnx_status = g_ort->CreateSessionOptions (&g_session_options);
        if (onnx_status != NULL)
        {
            g_ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
#ifdef _WIN32
        wchar_t model_path[1024];
        mbstowcs (model_path, params->file.c_str (), 1024);
        OrtStatus *onnx_status =
            g_ort->CreateSession (g_env, model_path, g_session_options, &g_session);
#else
        OrtStatus *onnx_status =
            g_ort->CreateSession (g_env, params->file.c_str (), g_session_options, &g_session);
#endif
        if (onnx_status != NULL)
        {
            g_ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        release ();
    }

    return res;
}

int predict (double *data, int data_len, double *output)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int release ()
{
    if (g_ort == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    if (g_session_options != NULL)
    {
        g_ort->ReleaseSessionOptions (g_session_options);
        g_session_options = NULL;
    }
    if (g_session != NULL)
    {
        g_ort->ReleaseSession (g_session);
        g_session = NULL;
    }
    if (g_env != NULL)
    {
        g_ort->ReleaseEnv (g_env);
        g_env = NULL;
    }
    g_ort = NULL;
    return (int)BrainFlowExitCodes::STATUS_OK;
}