#pragma once

#include "base_classifier.h"


class ConcentrationRegressionClassifier : public BaseClassifier
{
public:
    static std::shared_ptr<spdlog::logger> ml_logger;
    static int set_log_level (int log_level);
    static int set_log_file (char *log_file);

    ConcentrationRegressionClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
    }

    virtual ~ConcentrationRegressionClassifier ()
    {
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();

    // Classifier ml_logger should not be called from destructors, to ensure that there are safe log
    // methods Classifierml_logger still available but should be used only outside destructors
    template <typename Arg1, typename... Args>
    void safe_logger (spdlog::level::level_enum log_level, const char *fmt, const Arg1 &arg1,
        const Args &... args)
    {
        if (!skip_logs)
        {
            ConcentrationRegressionClassifier::ml_logger->log (log_level, fmt, arg1, args...);
        }
    }

    template <typename T> void safe_logger (spdlog::level::level_enum log_level, const T &msg)
    {
        if (!skip_logs)
        {
            ConcentrationRegressionClassifier::ml_logger->log (log_level, msg);
        }
    }
};
