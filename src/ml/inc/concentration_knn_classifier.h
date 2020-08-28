#pragma once

#include <queue>
#include <vector>

#include "base_classifier.h"
#include "knn_entry.h"


class ConcentrationKNNClassifier : public BaseClassifier
{
public:
    ConcentrationKNNClassifier (struct BrainFlowModelParams params) : BaseClassifier (params)
    {
        num_neighbors = 20;
        try
        {
            num_neighbors = std::stoi (params.other_info);
        }
        catch (const std::exception &e)
        {
            // do nothing
        }
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

    std::priority_queue<KNNEntry> get_k_smallest (int start_pos, int end_pos);
    double get_score (std::vector<std::priority_queue<KNNEntry>> chunks);
};
