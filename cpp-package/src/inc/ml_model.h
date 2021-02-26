#pragma once

#include <stdlib.h>
#include <string>

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

public:
    MLModel (struct BrainFlowModelParams params);
    ~MLModel ()
    {
    }

    /// redirect logger to a file
    static void set_log_file (std::string log_file);
    /// enable ML logger with LEVEL_INFO
    static void enable_ml_logger ();
    /// disable ML loggers
    static void disable_ml_logger ();
    /// enable ML logger with LEVEL_TRACE
    static void enable_dev_ml_logger ();
    /// set log level
    static void set_log_level (int log_level);

    /// initialize classifier, should be called first
    void prepare ();
    /// calculate metric from data
    double predict (double *data, int data_len);
    /// release classifier
    void release ();
};
