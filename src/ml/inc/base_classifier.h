#pragma once

#include <deque>
#include <memory>
#include <vector>

#include "brainflow_constants.h"
#include "brainflow_model_params.h"
#include "spdlog/spdlog.h"

#define DEFAULT_MOVING_AVERAGE_WINDOW 5

class BaseClassifier
{
public:
    struct BrainFlowModelParams params;
    bool skip_logs;

    static std::shared_ptr<spdlog::logger> ml_logger;
    static int set_log_level (int log_level);
    static int set_log_file (const char *log_file);

    BaseClassifier (struct BrainFlowModelParams model_params) : params (model_params)
    {
        skip_logs = false;
        use_moving_average = false;
        moving_average_window = 0;
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

    int prepare ();
    int predict (double *data, int data_len, double *output, int *output_len);
    int release ();

protected:
    bool use_moving_average;
    int moving_average_window;
    std::deque<std::vector<double>> window_data;

    void parse_moving_average_params ();
    void reset_moving_average ();
    void apply_moving_average (double *output, int *output_len);

    virtual int prepare_classifier ()
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
    virtual int calculate (double *data, int data_len, double *output, int *output_len) = 0;
    virtual int release_classifier ()
    {
        return (int)BrainFlowExitCodes::STATUS_OK;
    }
};
