#pragma once

#include "dyn_lib_classifier.h"
#include "env_vars.h"
#include "get_dll_dir.h"


class ONNXClassifier : public DynLibClassifier
{
public:
    ONNXClassifier (struct BrainFlowModelParams params) : DynLibClassifier (params)
    {
    }

protected:
    std::string get_dyn_lib_path ()
    {
        char onnxlib_dir[1024];
        bool res = get_dll_path (onnxlib_dir);
        std::string onnxlib_path = "";
#ifdef _WIN32
        std::string lib_name;
        if (sizeof (void *) == 4)
        {
            lib_name = "brainflow_onnx_x32.dll";
        }
        else
        {
            lib_name = "brainflow_onnx_x64.dll";
        }
#elif defined(__APPLE__)
        std::string lib_name = "libbrainflow_onnx.dylib";
#else
        std::string lib_name = "libbrainflow_onnx.so";
#endif
        if (res)
        {
            onnxlib_path = std::string (onnxlib_dir) + lib_name;
            append_to_search_path (onnxlib_dir);
        }
        else
        {
            onnxlib_path = lib_name;
        }

        safe_logger (spdlog::level::info, "use dyn lib: {}", onnxlib_path.c_str ());
        return onnxlib_path;
    }
};
