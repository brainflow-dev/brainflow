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

    std::vector<std::priority_queue<KNNEntry>> chunks (num_chunks);
    int chunk_size = dataset.size () / num_chunks;

#pragma omp parallel for
    for (int i = 0; i < num_chunks; i++)
    {
        int start_pos = i * chunk_size;
        int end_pos = (i + 1) * chunk_size;
        if (i == num_chunks - 1)
        {
            end_pos = dataset.size ();
        }
        chunks.at (i) = get_k_smallest (start_pos, end_pos);
    }

    double score = get_score (chunks);
    *output = score;

    return (int)BrainFlowExitCodes::STATUS_OK;
}

std::priority_queue<KNNEntry> ConcentrationKNNClassifier::get_k_smallest (
    int start_pos, int end_pos)
{
    std::priority_queue<KNNEntry> queue;
    for (auto data : dataset)
    {
        if ((queue.size () >= num_neighbors) && (queue.top () > data))
        {
            queue.push (data);
            queue.pop ();
        }
        else
        {
            if (queue.size () < num_neighbors)
            {
                queue.push (data);
            }
        }
    }
    return queue;
}

double ConcentrationKNNClassifier::get_score (std::vector<std::priority_queue<KNNEntry>> chunks)
{
    // merge all queues to the single one
    std::priority_queue<KNNEntry> queue;
    for (int i = 0; i < chunks.size (); i++)
    {
        while (!chunks.at (i).empty ())
        {
            KNNEntry data = chunks.at (i).top ();
            chunks.at (i).pop ();
            if ((queue.size () >= num_neighbors) && (queue.top () > data))
            {
                queue.push (data);
                queue.pop ();
            }
            else
            {
                if (queue.size () < num_neighbors)
                {
                    queue.push (data);
                }
            }
        }
    }
    int num_ones = 0;
    while (!queue.empty ())
    {
        KNNEntry data = queue.top ();
        queue.pop ();
        if (data.value == 1)
        {
            num_ones++;
        }
    }
    return ((double)num_ones) / num_neighbors;
}

int ConcentrationKNNClassifier::release ()
{
    dataset.clear ();
    return (int)BrainFlowExitCodes::STATUS_OK;
}
