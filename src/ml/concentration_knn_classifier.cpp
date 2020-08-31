#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_knn_classifier.h"
#include "focus_dataset.h"


int ConcentrationKNNClassifier::prepare ()
{
    if (!params.other_info.empty ())
    {
        try
        {
            num_neighbors = std::stoi (params.other_info);
        }
        catch (const std::exception &e)
        {
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
        }
    }
    if ((num_neighbors < 1) || (num_neighbors > 100))
    {
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int dataset_len = sizeof (brainflow_focus_y) / sizeof (brainflow_focus_y[0]);
    for (int i = 0; i < dataset_len; i++)
    {
        KNNEntry entry (brainflow_focus_x[i], brainflow_focus_y[i], 10);
        // decrease weight for stddev, 0.2 - experimental vlaue
        for (int j = 5; j < 10; j++)
        {
            entry.feature_vector[j] *= 0.2;
        }
        dataset.push_back (entry);
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    if (dataset.empty ())
    {
        return (int)BrainFlowExitCodes::CLASSIFIER_IS_NOT_PREPARED_ERROR;
    }

    KNNEntry sample_to_predict (data, 0, data_len);

    int num_chunks = 1; // todo num_chunks == num threads
    int chunk_size = dataset.size () / num_chunks;
    // todo parallel using c++ threads
    for (int i = 0; i < dataset.size (); i++)
    {
        dataset.at (i).set_distance (sample_to_predict);
    }

    std::vector<std::vector<KNNEntry>> chunks (num_chunks);

    // find num_neighbors elements in each chunk
    // todo parallel using c++ threads
    for (int i = 0; i < num_chunks; i++)
    {
        int start_pos = i * chunk_size;
        int end_pos = (i + 1) * chunk_size;
        if (i == num_chunks - 1)
        {
            end_pos = dataset.size () - 1;
        }
        std::vector<KNNEntry> search_vector (
            dataset.begin () + start_pos, dataset.begin () + end_pos);
        int num_points = std::min (num_neighbors, end_pos - start_pos);
        std::nth_element (
            search_vector.begin (), search_vector.begin () + num_points, search_vector.end ());
        for (int j = 0; j < num_points; j++)
        {
            chunks[i].push_back (search_vector[j]);
        }
    }

    // join chunks and update mins
    std::vector<KNNEntry> all_sorted_data;
    for (int i = 0; i < num_chunks; i++)
    {
        for (int j = 0; j < chunks[i].size (); j++)
        {
            all_sorted_data.push_back (chunks[i][j]);
        }
    }
    std::nth_element (
        all_sorted_data.begin (), all_sorted_data.begin () + num_neighbors, all_sorted_data.end ());
    // calc probability as num ones / num neighbors
    double num_ones = 0.0;
    for (int i = 0; i < num_neighbors; i++)
    {
        if (all_sorted_data[i].value == 1)
        {
            num_ones = num_ones + 1;
        }
    }

    double score = num_ones / num_neighbors;
    *output = score;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::release ()
{
    dataset.clear ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}
