#include "onnx_classifier.h"
#include "env_vars.h"
#include "get_dll_dir.h"

bool ONNXClassifier::env_set = false;

ONNXClassifier::ONNXClassifier (struct BrainFlowModelParams params) : DynLibClassifier (params)
{
}

ONNXClassifier::~ONNXClassifier ()
{
}

std::string ONNXClassifier::get_dyn_lib_path ()
{
    char onnxlib_dir[1024];
    bool res = get_dll_path (onnxlib_dir);
    std::string onnxlib_path = "";
#ifdef _WIN32
    std::string lib_name;
    if (sizeof (void *) == 4)
    {
        lib_name = "brainflow_onnx32.dll";
    }
    else
    {
        lib_name = "brainflow_onnx.dll";
    }
#elif defined(__APPLE__)
    std::string lib_name = "libbrainflow_onnx.dylib";
#else
    std::string lib_name = "libbrainflow_onnx.so";
#endif
    if (res)
    {
        onnxlib_path = std::string (onnxlib_dir) + lib_name;
        if (!ONNXClassifier::env_set)
        {
            append_to_search_path (onnxlib_dir);
            ONNXClassifier::env_set = true;
        }
    }
    else
    {
        onnxlib_path = lib_name;
    }
    safe_logger (spdlog::level::trace, "Current search path: {}", get_search_path ());
    safe_logger (spdlog::level::info, "use dyn lib: {}", onnxlib_path.c_str ());
    return onnxlib_path;
}