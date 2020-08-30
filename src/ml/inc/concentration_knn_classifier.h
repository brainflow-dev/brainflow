#pragma once

#include <vector>

#include "base_classifier.h"
#include "knn_entry.h"


class ConcentrationKNNClassifier : public BaseClassifier
{
public:
    ConcentrationKNNClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        num_neighbors = 20;
    }

    virtual ~ConcentrationKNNClassifier ()
    {
        release ();
    }

    virtual int prepare ();
    virtual int predict (double *data, int data_len, double *output);
    virtual int release ();

private:
    std::vector<KNNEntry> dataset;
    int num_neighbors;
};
