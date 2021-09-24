#pragma once

#include "brainflow_model_params.h"
#include "spdlog/spdlog.h"

class BaseClassifier
{
protected:
    struct BrainFlowModelParams params;
    bool skip_logs;

public:
    static std::shared_ptr<spdlog::logger> ml_logger;
    static int set_log_level (int log_level);
    static int set_log_file (const char *log_file);

    BaseClassifier (struct BrainFlowModelParams model_params) : params (model_params)
    {
        skip_logs = false;
    }

    virtual ~BaseClassifier ()
    {
        skip_logs = true;
    }
    // Classifier ml_logger should not be called from destructors, to ensure that there are safe log
    // methods Classifierml_logger still available but should be used only outside destructors
    template <typename Arg1, typename... Args>
    // clang-format off
    void safe_logger (
        spdlog::level::level_enum log_level, const char *fmt, const Arg1 &arg1, const Args &... args)
    // clang-format on
    {
        if (!skip_logs)
        {
            BaseClassifier::ml_logger->log (log_level, fmt, arg1, args...);
        }
    }

    template <typename T>
    void safe_logger (spdlog::level::level_enum log_level, const T &msg)
    {
        if (!skip_logs)
        {
            BaseClassifier::ml_logger->log (log_level, msg);
        }
    }

    virtual int prepare () = 0;
    virtual int predict (double *data, int data_len, double *output) = 0;
    virtual int release () = 0;
};
