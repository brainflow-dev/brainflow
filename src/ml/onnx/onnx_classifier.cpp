#include "onnx_classifier.h"
#include "brainflow_constants.h"
#include "get_dll_dir.h"

#include <iostream>

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

int OnnxClassifier::prepare ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (params.file.empty ())
    {
        safe_logger (spdlog::level::err, "file with onnx model is not provided");
        res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = load_api ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = get_input_info ();
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        res = get_output_info ();
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        release ();
    }
    return res;
}


int OnnxClassifier::predict (double *data, int data_len, double *output, int *output_len)
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int OnnxClassifier::release ()
{
    if ((allocator != NULL) && (ort != NULL))
    {
        for (const char *node_name : input_node_names)
        {
            ort->AllocatorFree (
                allocator, const_cast<void *> (reinterpret_cast<const void *> (node_name)));
        }
        for (const char *node_name : output_node_names)
        {
            ort->AllocatorFree (
                allocator, const_cast<void *> (reinterpret_cast<const void *> (node_name)));
        }
    }
    if ((session_options != NULL) && (ort != NULL))
    {
        ort->ReleaseSessionOptions (session_options);
        session_options = NULL;
    }
    if ((session != NULL) && (ort != NULL))
    {
        ort->ReleaseSession (session);
        session = NULL;
    }
    if ((env != NULL) && (ort != NULL))
    {
        ort->ReleaseEnv (env);
        env = NULL;
    }
    ort = NULL;
    if (dll_loader != NULL)
    {
        dll_loader->free_library ();
        delete dll_loader;
        dll_loader = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

std::string OnnxClassifier::get_onnxlib_path ()
{
    char onnxlib_dir[1024];
    bool res = get_dll_path (onnxlib_dir);
    std::string onnxlib_path = "";
#ifdef _WIN32
    std::string lib_name;
    if (sizeof (void *) == 4)
    {
#if defined(__arm__) || defined(_M_ARM64) || defined(_M_ARM) || defined(__aarch64__)
        lib_name = "onnxruntime_arm.dll";
#else
        lib_name = "onnxruntime_x86.dll";
#endif
    }
    else
    {
#if defined(__arm__) || defined(_M_ARM64) || defined(_M_ARM) || defined(__aarch64__)
        lib_name = "onnxruntime_arm64.dll";
#else
        lib_name = "onnxruntime_x64.dll";
#endif
    }
#elif defined(__APPLE__)
#if defined(__arm__) || defined(_M_ARM64) || defined(_M_ARM) || defined(__aarch64__)
    std::string lib_name = "libonnxruntime_arm64.dylib";
#else
    std::string lib_name = "libonnxruntime_x64.dylib";
#endif
#else
#if defined(__arm__) || defined(_M_ARM64) || defined(_M_ARM) || defined(__aarch64__)
    std::string lib_name = "libonnxruntime_arm64.so";
#else
    std::string lib_name = "libonnxruntime_x64.so";
#endif
#endif
    if (res)
    {
        onnxlib_path = std::string (onnxlib_dir) + lib_name;
    }
    else
    {
        onnxlib_path = lib_name;
    }
    return onnxlib_path;
}

int OnnxClassifier::load_api ()
{
    if (dll_loader != NULL)
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    std::string onnxlib_path = get_onnxlib_path ();
    const OrtApiBase *(*OrtGetApiBase) (void) = NULL;
    dll_loader = new DLLLoader (onnxlib_path.c_str ());
    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library: {}", onnxlib_path);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtGetApiBase = (const OrtApiBase *(*)(void))dll_loader->get_address ("OrtGetApiBase");
        if (OrtGetApiBase == NULL)
        {
            safe_logger (spdlog::level::err, "failed to get function address for OrtGetApiBase");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        ort = OrtGetApiBase ()->GetApi (7);
        if (ort == NULL)
        {
            safe_logger (spdlog::level::err, "Ort GetApi failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtStatus *onnx_status = ort->CreateEnvWithCustomLogger (
            log_onnx_msg, (void *)this, ORT_LOGGING_LEVEL_VERBOSE, "brainflow_onnx_lib", &env);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "CreateEnv failed: {}", msg);
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
            safe_logger (spdlog::level::err, "CreateSessionOptions failed: {}", msg);
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
            safe_logger (spdlog::level::err, "CreateSession failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtStatus *onnx_status = ort->GetAllocatorWithDefaultOptions (&allocator);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "GetAllocatorWithDefaultOptions failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete dll_loader;
        dll_loader = NULL;
    }

    return res;
}

int OnnxClassifier::get_input_info ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    size_t num_input_nodes = 0;
    OrtTypeInfo *typeinfo = NULL;
    size_t num_dims = 0;
    char *input_name = NULL;
    const OrtTensorTypeAndShapeInfo *tensor_info;

    // input count
    OrtStatus *onnx_status = ort->SessionGetInputCount (session, &num_input_nodes);
    if (onnx_status != NULL)
    {
        const char *msg = ort->GetErrorMessage (onnx_status);
        safe_logger (spdlog::level::err, "SessionGetInputCount failed: {}", msg);
        ort->ReleaseStatus (onnx_status);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (num_input_nodes != 1)
        {
            safe_logger (spdlog::level::err, "Only one input node is supported");
            res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }

    // input node types
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->SessionGetInputTypeInfo (session, 0, &typeinfo);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "SessionGetInputTypeInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->CastTypeInfoToTensorInfo (typeinfo, &tensor_info);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "CastTypeInfoToTensorInfo failed: {}", msg);
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
            safe_logger (spdlog::level::err, "GetTensorElementType failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            safe_logger (spdlog::level::info, "input type is: {}", (int)input_type);
        }
    }

    // check input dims
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->GetDimensionsCount (tensor_info, &num_dims);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "GetDimensionsCount failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            safe_logger (spdlog::level::info, "num dims is: {}", num_dims);
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        input_node_dims.resize (num_dims);
        onnx_status =
            ort->GetDimensions (tensor_info, (int64_t *)input_node_dims.data (), num_dims);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "GetDimensions failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            for (size_t j = 0; j < num_dims; j++)
            {
                safe_logger (spdlog::level::info, "Input Dim {} size {}", j, input_node_dims[j]);
            }
        }
    }

    // input name
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->SessionGetInputName (session, 0, allocator, &input_name);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "SessionGetInputName failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            input_node_names.resize (1);
            input_node_names[0] = input_name;
        }
    }

    if (typeinfo != NULL)
    {
        ort->ReleaseTypeInfo (typeinfo);
    }

    return res;
}

int OnnxClassifier::get_output_info ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    size_t num_output_nodes = 0;
    size_t node_number = 0;
    OrtTypeInfo *typeinfo = NULL;
    size_t num_dims = 0;
    char *output_name = NULL;
    bool node_found = false;
    const OrtTensorTypeAndShapeInfo *tensor_info;

    // output count
    OrtStatus *onnx_status = ort->SessionGetOutputCount (session, &num_output_nodes);
    if (onnx_status != NULL)
    {
        const char *msg = ort->GetErrorMessage (onnx_status);
        safe_logger (spdlog::level::err, "SessionGetOutputCount failed: {}", msg);
        ort->ReleaseStatus (onnx_status);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        for (node_number = 0; node_number < num_output_nodes; node_number++)
        {
            // output name
            if (res == (int)BrainFlowExitCodes::STATUS_OK)
            {
                onnx_status =
                    ort->SessionGetOutputName (session, node_number, allocator, &output_name);
                if (onnx_status != NULL)
                {
                    const char *msg = ort->GetErrorMessage (onnx_status);
                    safe_logger (spdlog::level::err, "SessionGetOutputName failed: {}", msg);
                    ort->ReleaseStatus (onnx_status);
                    res = (int)BrainFlowExitCodes::GENERAL_ERROR;
                }
                else
                {
                    safe_logger (spdlog::level::info, "found output node: {}", output_name);
                    if ((num_output_nodes == 1) ||
                        (strcmp (params.output_name.c_str (), output_name) == 0))
                    {
                        output_node_names.resize (1);
                        output_node_names[0] = output_name;
                        node_found = true;
                        break;
                    }
                    else
                    {
                        ort->AllocatorFree (allocator, output_name);
                    }
                }
            }
        }
    }
    if (!node_found)
    {
        safe_logger (spdlog::level::err,
            "Model has multiple output nodes, you need to provide correct node name via "
            "BrainFlowModelParams.output_name");
        res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // output node types
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->SessionGetOutputTypeInfo (session, node_number, &typeinfo);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "SessionGetOutputTypeInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->CastTypeInfoToTensorInfo (typeinfo, &tensor_info);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "CastTypeInfoToTensorInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->GetTensorElementType (tensor_info, &output_type);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "GetTensorElementType failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            safe_logger (spdlog::level::info, "output type is: {}", (int)input_type);
        }
    }

    // check output dims
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->GetDimensionsCount (tensor_info, &num_dims);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "GetDimensionsCount failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            safe_logger (spdlog::level::info, "num dims is: {}", num_dims);
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        output_node_dims.resize (num_dims);
        onnx_status =
            ort->GetDimensions (tensor_info, (int64_t *)output_node_dims.data (), num_dims);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "GetDimensions failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            for (size_t j = 0; j < num_dims; j++)
            {
                safe_logger (spdlog::level::info, "Output Dim {} size {}", j, output_node_dims[j]);
            }
        }
    }

    if (typeinfo != NULL)
    {
        ort->ReleaseTypeInfo (typeinfo);
    }

    return res;
}