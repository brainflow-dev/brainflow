#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_knn_classifier.h"
#include "focus_dataset.h"

#ifdef _OPENMP
#include <omp.h>
#endif


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
        for (int i = 5; i < 10; i++)
        {
            entry.feature_vector[i] *= 0.2;
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

#pragma omp parallel for
    for (int i = 0; i < dataset.size (); i++)
    {
        dataset.at (i).set_distance (sample_to_predict);
    }

#ifdef _OPENMP
    int num_chunks = omp_get_num_threads ();
#else
    int num_chunks = 1;
#endif
    int chunk_size = dataset.size () / num_chunks;

    std::vector<std::vector<KNNEntry>> chunks (num_chunks);

    // find num_neighbors elements in each chunk
#pragma omp parallel for
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
        std::nth_element (
            search_vector.begin (), search_vector.begin () + num_neighbors, search_vector.end ());
        chunks.at (i) = search_vector;
    }

    // join chunks and update mins
    std::vector<KNNEntry> all_sorted_data;
    for (int i = 0; i < num_chunks; i++)
    {
        all_sorted_data.insert (
            all_sorted_data.end (), chunks.at (i).begin (), chunks.at (i).begin () + num_neighbors);
    }
    std::nth_element (
        all_sorted_data.begin (), all_sorted_data.begin () + num_neighbors, all_sorted_data.end ());
    // calc probability as num ones / num neighbors
    double num_ones = 0.0;
    for (int i = 0; i < num_neighbors; i++)
    {
        if (all_sorted_data.at (i).value == 1)
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
