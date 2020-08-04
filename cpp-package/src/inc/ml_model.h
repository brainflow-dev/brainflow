#pragma once

#include <stdlib.h>

// include it here to allow user include only this single file
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "ml_module.h"

/// Calculates different metrics from raw data
class MLModel
{
private:
    int metric;
    int classifier;

public:
    // clang-format off
    MLModel (int metric, int classifier);
    ~MLModel ();

    /// initialize classifier, should be called first
    void prepare ();
    /// calculate metric from data
    double predict (double *data, int data_len);
    /// release classifier
    void release ();
    // clang-format on
};
