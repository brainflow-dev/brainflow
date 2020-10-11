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
        skip_logs=true;
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();
};
