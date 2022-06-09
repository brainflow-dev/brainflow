#include "onnx_classifier.h"
#include "brainflow_constants.h"
#include "get_dll_dir.h"


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
    if (dll_loader != NULL)
    {
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }

    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (params.file.empty ())
    {
        safe_logger (spdlog::level::err, "file with onnx model is not provided");
        res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    if (params.max_array_size < 1)
    {
        safe_logger (spdlog::level::err, "max array size param is invalid");
        res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    std::string onnxlib_path = get_onnxlib_path ();
    dll_loader = new DLLLoader (onnxlib_path.c_str ());
    if (!dll_loader->load_library ())
    {
        safe_logger (spdlog::level::err, "Failed to load library: {}", onnxlib_path);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
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
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    if (ort == NULL)
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    if ((data == NULL) || (data_len < 1) || (output == NULL) || (output_len == NULL))
    {
        safe_logger (spdlog::level::err, "invalid input arguments");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // todo add support for ints and float16
    float *float_data = NULL;
    if (input_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
    {
        float_data = new float[data_len];
        for (int i = 0; i < data_len; i++)
        {
            float_data[i] = (float)data[i];
        }
    }
    else if (input_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE)
    {
        // no need to convert
    }
    else
    {
        safe_logger (
            spdlog::level::err, "only float and double input types are currently supported");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // create input tensor object from data values
    OrtMemoryInfo *memory_info = NULL;
    OrtValue *input_tensor = NULL;
    OrtValue *output_tensor = NULL;
    OrtStatus *onnx_status =
        ort->CreateCpuMemoryInfo (OrtArenaAllocator, OrtMemTypeDefault, &memory_info);
    if (onnx_status != NULL)
    {
        const char *msg = ort->GetErrorMessage (onnx_status);
        safe_logger (spdlog::level::err, "CreateCpuMemoryInfo failed: {}", msg);
        ort->ReleaseStatus (onnx_status);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else if (memory_info == NULL)
    {
        safe_logger (spdlog::level::err, "CreateCpuMemoryInfo failed");
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        if (input_type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)
        {
            onnx_status = ort->CreateTensorWithDataAsOrtValue (memory_info, float_data,
                data_len * sizeof (float), input_node_dims.data (), input_node_dims.size (),
                ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, &input_tensor);
        }
        else
        {
            onnx_status = ort->CreateTensorWithDataAsOrtValue (memory_info, data,
                data_len * sizeof (double), input_node_dims.data (), input_node_dims.size (),
                ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE, &input_tensor);
        }
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "CreateTensorWithDataAsOrtValue failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (input_tensor == NULL)
        {
            safe_logger (spdlog::level::err, "CreateTensorWithDataAsOrtValue failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        int is_tensor = 0;
        onnx_status = ort->IsTensor (input_tensor, &is_tensor);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "IsTensor failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            if (!is_tensor)
            {
                safe_logger (spdlog::level::err, "Input isnt a tensor");
                res = (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
        }
    }

    // score model & input tensor, get back output tensor
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->Run (session, NULL, input_node_names.data (),
            (const OrtValue *const *)&input_tensor, 1, output_node_names.data (), 1,
            &output_tensor);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "Run failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (output_tensor == NULL)
        {
            safe_logger (spdlog::level::err, "Run failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        int is_tensor = 0;
        onnx_status = ort->IsTensor (output_tensor, &is_tensor);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "IsTensor failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            if (!is_tensor)
            {
                safe_logger (spdlog::level::err, "Output isnt a tensor");
                res = (int)BrainFlowExitCodes::GENERAL_ERROR;
            }
        }
    }

    // Get pointer to output tensor float values
    size_t output_size = 1;
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        for (int64_t output_dim : output_node_dims)
        {
            output_size *= output_dim;
        }
        if (output_size > (uint64_t)params.max_array_size)
        {
            safe_logger (spdlog::level::warn, "output is bigger than allocated array");
            output_size = params.max_array_size;
        }

        void *output_tensor_data = NULL;
        onnx_status = ort->GetTensorMutableData (output_tensor, &output_tensor_data);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "IsTensor failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (output_tensor_data == NULL)
        {
            safe_logger (spdlog::level::err, "GetTensorMutableData failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            *output_len = (int)output_size;
            switch (output_type)
            {
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED:
                {
                    safe_logger (spdlog::level::trace, "undefined output type");
                    res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
                {
                    float *output_data = (float *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
                {
                    uint8_t *output_data = (uint8_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8:
                {
                    int8_t *output_data = (int8_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16:
                {
                    uint16_t *output_data = (uint16_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16:
                {
                    int16_t *output_data = (int16_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
                {
                    int32_t *output_data = (int32_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
                {
                    int64_t *output_data = (int64_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_STRING:
                {
                    safe_logger (spdlog::level::err, "string output type is not supported");
                    res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16:
                {
                    safe_logger (spdlog::level::err, "float16 output type is not supported");
                    res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
                {
                    double *output_data = (double *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32:
                {
                    uint32_t *output_data = (uint32_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64:
                {
                    uint64_t *output_data = (uint64_t *)output_tensor_data;
                    for (uint64_t i = 0; i < output_size; i++)
                    {
                        output[i] = (double)output_data[i];
                    }
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_COMPLEX64:
                {
                    safe_logger (spdlog::level::err, "complex64 output type is not supported");
                    res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_COMPLEX128:
                {
                    safe_logger (spdlog::level::err, "complex128 output type is not supported");
                    res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_BFLOAT16:
                {
                    safe_logger (spdlog::level::err, "bfloat16 output type is not supported");
                    res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
                    break;
                }
                default:
                {
                    safe_logger (spdlog::level::err, "unknown output type");
                    res = (int)BrainFlowExitCodes::STATUS_OK;
                }
            }
        }
    }

    if (output_tensor != NULL)
    {
        ort->ReleaseValue (output_tensor);
    }
    if (input_tensor != NULL)
    {
        ort->ReleaseValue (input_tensor);
    }
    if (memory_info != NULL)
    {
        ort->ReleaseMemoryInfo (memory_info);
    }
    if (float_data != NULL)
    {
        delete[] float_data;
    }

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int OnnxClassifier::release ()
{
    if ((allocator != NULL) && (ort != NULL))
    {
        for (const char *node_name : input_node_names)
        {
            OrtStatus *status = ort->AllocatorFree (
                allocator, const_cast<void *> (reinterpret_cast<const void *> (node_name)));
        }
        for (const char *node_name : output_node_names)
        {
            OrtStatus *status = ort->AllocatorFree (
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
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    const OrtApiBase *(*OrtGetApiBase) (void) = NULL;
    OrtGetApiBase = (const OrtApiBase *(*)(void))dll_loader->get_address ("OrtGetApiBase");
    if (OrtGetApiBase == NULL)
    {
        safe_logger (spdlog::level::err, "failed to get function address for OrtGetApiBase");
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        ort = OrtGetApiBase ()->GetApi (ORT_API_VERSION);
        if (ort == NULL)
        {
            safe_logger (spdlog::level::err, "Ort GetApi failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        OrtStatus *onnx_status = ort->CreateEnvWithCustomLogger ((OrtLoggingFunction)log_onnx_msg,
            (void *)this, ORT_LOGGING_LEVEL_VERBOSE, "brainflow_onnx_lib", &env);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "CreateEnv failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (env == NULL)
        {
            safe_logger (spdlog::level::err, "CreateEnvWithCustomLogger failed");
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
        else if (session_options == NULL)
        {
            safe_logger (spdlog::level::err, "CreateSessionOptions failed");
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
        else if (session == NULL)
        {
            safe_logger (spdlog::level::err, "CreateSessionOptions failed");
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
        else if (allocator == NULL)
        {
            safe_logger (spdlog::level::err, "GetAllocatorWithDefaultOptions failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }

    return res;
}

int OnnxClassifier::get_input_info ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    size_t num_input_nodes = 0;
    OrtTypeInfo *type_info = NULL;
    size_t num_dims = 0;
    char *input_name = NULL;
    const OrtTensorTypeAndShapeInfo *tensor_info = NULL;

    // input count
    OrtStatus *onnx_status = ort->SessionGetInputCount (session, &num_input_nodes);
    if (onnx_status != NULL)
    {
        const char *msg = ort->GetErrorMessage (onnx_status);
        safe_logger (spdlog::level::err, "SessionGetInputCount failed: {}", msg);
        ort->ReleaseStatus (onnx_status);
        res = (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    else if (num_input_nodes == 0)
    {
        safe_logger (spdlog::level::err, "SessionGetInputCount failed");
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
        onnx_status = ort->SessionGetInputTypeInfo (session, 0, &type_info);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "SessionGetInputTypeInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (type_info == NULL)
        {
            safe_logger (spdlog::level::err, "SessionGetInputTypeInfo failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->CastTypeInfoToTensorInfo (type_info, &tensor_info);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "CastTypeInfoToTensorInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (tensor_info == NULL)
        {
            safe_logger (spdlog::level::err, "CastTypeInfoToTensorInfo failed");
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
        else if (input_name == NULL)
        {
            safe_logger (spdlog::level::err, "SessionGetInputName failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else
        {
            input_node_names.resize (1);
            input_node_names[0] = input_name;
        }
    }

    if (type_info != NULL)
    {
        ort->ReleaseTypeInfo (type_info);
    }

    return res;
}

int OnnxClassifier::get_output_info ()
{
    int res = (int)BrainFlowExitCodes::STATUS_OK;
    size_t num_output_nodes = 0;
    size_t node_number = 0;
    OrtTypeInfo *type_info = NULL;
    size_t num_dims = 0;
    char *output_name = NULL;
    bool node_found = false;
    const OrtTensorTypeAndShapeInfo *tensor_info = NULL;

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
                    if (((num_output_nodes == 1) ||
                            (strcmp (params.output_name.c_str (), output_name) == 0)) ||
                        ((params.output_name.empty ()) &&
                            (strcmp (output_name, "output_probability") == 0)) ||
                        ((params.output_name.empty ()) &&
                            (strcmp (output_name, "probabilities") == 0)))
                    {
                        output_node_names.resize (1);
                        output_node_names[0] = output_name;
                        node_found = true;
                        break;
                    }
                    else
                    {
                        onnx_status = ort->AllocatorFree (allocator, output_name);
                    }
                }
            }
        }
    }
    if (!node_found)
    {
        safe_logger (spdlog::level::err,
            "Model has multiple output nodes, you need to provide correct node name via "
            "BrainFlowModelParams.output_name, you can use https://netron.app/ to inspect the "
            "model");
        res = (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    // output node types
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->SessionGetOutputTypeInfo (session, node_number, &type_info);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "SessionGetOutputTypeInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (type_info == NULL)
        {
            safe_logger (spdlog::level::err, "SessionGetOutputTypeInfo failed");
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
    }
    if (res == (int)BrainFlowExitCodes::STATUS_OK)
    {
        onnx_status = ort->CastTypeInfoToTensorInfo (type_info, &tensor_info);
        if (onnx_status != NULL)
        {
            const char *msg = ort->GetErrorMessage (onnx_status);
            safe_logger (spdlog::level::err, "CastTypeInfoToTensorInfo failed: {}", msg);
            ort->ReleaseStatus (onnx_status);
            res = (int)BrainFlowExitCodes::GENERAL_ERROR;
        }
        else if (tensor_info == NULL)
        {
            safe_logger (spdlog::level::err,
                "CastTypeInfoToTensorInfo failed, make sure that ZipMap is disabled in your "
                "model.");
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

    if (type_info != NULL)
    {
        ort->ReleaseTypeInfo (type_info);
    }

    return res;
}