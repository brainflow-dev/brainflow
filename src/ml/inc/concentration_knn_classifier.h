#pragma once

#include <vector>

#include "base_classifier.h"
#include "focus_point.h"

#include "kdtree.h"


class ConcentrationKNNClassifier : public BaseClassifier
{
public:
    ConcentrationKNNClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        num_neighbors = 5;
    }

    virtual ~ConcentrationKNNClassifier ()
    {
        skip_logs = true;
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();

private:
    std::vector<std::vector<FocusPoint>> datasets;
    std::vector<kdt::KDTree<FocusPoint> *> kdtrees;
    int num_neighbors;
};
