#pragma once

#include <stdlib.h>

// include it here to allow user include only this single file
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "brainflow_model_params.h"
#include "ml_module.h"


/// Calculates different metrics from raw data
class MLModel
{
private:
    struct BrainFlowModelParams params;
    std::string serialized_params;
    static void set_log_level (int log_level);
    static void set_log_file (char *log_file);
public:
    // clang-format off
    MLModel (struct BrainFlowModelParams params);
    ~MLModel ();

    /// enable BrainFlow logger with LEVEL_INFO
    static void enable_board_logger ();
    /// disable BrainFlow loggers
    static void disable_board_logger ();
    /// enable BrainFlow logger with LEVEL_TRACE
    static void enable_dev_board_logger ();

    /// initialize classifier, should be called first
    void prepare ();
    /// calculate metric from data
    double predict (double *data, int data_len);
    /// release classifier
    void release ();
    // clang-format on
};
