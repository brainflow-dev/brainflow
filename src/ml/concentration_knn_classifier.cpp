#include <algorithm>
#include <cmath>
#include <future>
#include <stdlib.h>
#include <thread>

#include "brainflow_constants.h"
#include "concentration_knn_classifier.h"
#include "focus_dataset.h"


int ConcentrationKNNClassifier::prepare ()
{
    if (!kdtrees.empty ())
    {
        safe_logger (spdlog::level::err, "Classifier has already been prepared.");
        return (int)BrainFlowExitCodes::ANOTHER_CLASSIFIER_IS_PREPARED_ERROR;
    }
    if (!params.other_info.empty ())
    {
        try
        {
            num_neighbors = std::stoi (params.other_info);
        }
        catch (const std::exception &e)
        {
            safe_logger (spdlog::level::err, "Coudn't convert neighbors to integer value.");
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    if ((num_neighbors < 1) || (num_neighbors > 100))
    {
        safe_logger (spdlog::level::err, "You must pick from 1 to 100 neighbors.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int num_cores = std::thread::hardware_concurrency ();
    safe_logger (spdlog::level::debug, "Use {} threads for KNN calculation.", num_cores);
    int dataset_len = sizeof (brainflow_focus_y) / sizeof (brainflow_focus_y[0]);
    int data_per_thread = dataset_len / num_cores;

    for (int thread_num = 0; thread_num < num_cores; thread_num++)
    {
        int start_index = thread_num * data_per_thread;
        int stop_index = (thread_num + 1) * data_per_thread;
        if (thread_num == num_cores - 1)
        {
            stop_index = dataset_len;
        }
        if (start_index >= dataset_len)
        {
            break;
        }
        std::vector<FocusPoint> dataset;
        for (int i = start_index; i < stop_index; i++)
        {
            FocusPoint point (brainflow_focus_x[i], 10, brainflow_focus_y[i]);
            // decrease weight for stddev, 0.2 - experimental vlaue
            for (int j = 5; j < 10; j++)
            {
                point[j] *= 0.2;
            }
            dataset.push_back (point);
        }
        datasets.push_back (std::move (dataset));
        kdt::KDTree<FocusPoint> *kdtree = new kdt::KDTree<FocusPoint> (datasets.back ());
        kdtrees.push_back (kdtree);
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err, "All argument must not be null, and data_len must be 10");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    if (kdtrees.empty ())
    {
        safe_logger (spdlog::level::err, "Please prepare classifier with prepare method.");
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }

    double feature_vector[10] = {0.0};
    for (int i = 0; i < data_len; i++)
    {
        if (i >= 5)
        {
            feature_vector[i] = data[i] * 0.2;
        }
        else
        {
            feature_vector[i] = data[i];
        }
    }

    // find nearest per thread
    FocusPoint sample_to_predict (feature_vector, 10, 0.0);
    std::vector<std::future<std::vector<int>>> pool;
    for (int i = 0; i < kdtrees.size (); i++)
    {
        pool.emplace_back (std::async (
            std::launch::async,
            [this, &sample_to_predict](int i) {
                return this->kdtrees[i]->knnSearch (sample_to_predict, this->num_neighbors);
            },
            i));
    }

    // merge threads and find final neighbors
    std::vector<FocusPoint> merged_dataset;
    for (int i = 0; i < pool.size (); i++)
    {
        std::vector<int> ids = pool[i].get ();
        for (int j = 0; j < ids.size (); j++)
        {
            merged_dataset.push_back (datasets[i][ids[j]]);
        }
    }
    kdt::KDTree<FocusPoint> merged_kdtree (merged_dataset);

    // count ones
    const std::vector<int> knn_ids = merged_kdtree.knnSearch (sample_to_predict, num_neighbors);
    int num_ones = 0;
    for (int i = 0; i < knn_ids.size (); i++)
    {
        if (merged_dataset[knn_ids[i]].value == 1)
        {
            num_ones++;
        }
    }

    double score = ((double)num_ones) / num_neighbors;
    *output = score;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::release ()
{
    if (kdtrees.empty ())
    {
        safe_logger (spdlog::level::err, "Please prepare classifier with prepare method.");
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }
    for (auto p : kdtrees)
    {
        delete p;
    }
    kdtrees.clear ();
    for (auto dataset : datasets)
    {
        dataset.clear ();
    }
    datasets.clear ();
    safe_logger (spdlog::level::info, "Model has been cleared.");
    return (int)BrainFlowExitCodes::STATUS_OK;
}
