#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_knn_classifier.h"
#include "focus_dataset.h"
#include "spdlog/sinks/null_sink.h"

#define LOGGER_NAME "ml_logger"

#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
std::shared_ptr<spdlog::logger> ConcentrationKNNClassifier::ml_logger =
    spdlog::android_logger (LOGGER_NAME, "ml_ndk_logger");
#else
std::shared_ptr<spdlog::logger> ConcentrationKNNClassifier::ml_logger = spdlog::stderr_logger_mt (LOGGER_NAME);
#endif

int ConcentrationKNNClassifier::set_log_level (int level)
{
    int log_level = level;
    if (level > 6)
    {
        log_level = 6;
    }
    if (level < 0)
    {
        log_level = 0;
    }
    try
    {
        ConcentrationKNNClassifier::ml_logger->set_level (spdlog::level::level_enum (log_level));
        ConcentrationKNNClassifier::ml_logger->flush_on (spdlog::level::level_enum (log_level));
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::set_log_file (char *log_file)
{
#ifdef __ANDROID__
    ConcentrationKNNClassifier::ml_logger->error ("For Android set_log_file is unavailable");
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
#else
    try
    {
        spdlog::level::level_enum level = ConcentrationKNNClassifier::ml_logger->level ();
        ConcentrationKNNClassifier::ml_logger = spdlog::create<spdlog::sinks::null_sink_st> (
            "null_logger"); // to dont set logger to nullptr and avoid raice condition
        spdlog::drop (LOGGER_NAME);
        ConcentrationKNNClassifier::ml_logger = spdlog::basic_logger_mt (LOGGER_NAME, log_file);
        ConcentrationKNNClassifier::ml_logger->set_level (level);
        ConcentrationKNNClassifier::ml_logger->flush_on (level);
        spdlog::drop ("null_logger");
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}

int ConcentrationKNNClassifier::prepare ()
{
    if (kdtree != NULL)
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
        safe_logger (spdlog::level::err, "You must pick from 1-100 neighbors.");
        return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }

    int dataset_len = sizeof (brainflow_focus_y) / sizeof (brainflow_focus_y[0]);
    for (int i = 0; i < dataset_len; i++)
    {
        FocusPoint point (brainflow_focus_x[i], 10, brainflow_focus_y[i]);
        // decrease weight for stddev, 0.2 - experimental vlaue
        for (int j = 5; j < 10; j++)
        {
            point[j] *= 0.2;
        }
        dataset.push_back (point);
    }
    kdtree = new kdt::KDTree<FocusPoint> (dataset);
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationKNNClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err, "All argument must not be null, and data_len > 5.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    if (kdtree == NULL)
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

    FocusPoint sample_to_predict (feature_vector, 10, 0.0);
    const std::vector<int> knn_ids = kdtree->knnSearch (sample_to_predict, num_neighbors);
    int num_ones = 0;
    for (int i = 0; i < knn_ids.size (); i++)
    {
        if (dataset[knn_ids[i]].value == 1)
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
    delete kdtree;
    kdtree = NULL;
    dataset.clear ();
    safe_logger (spdlog::level::err, "Model has been cleared.");
    return (int)BrainFlowExitCodes::STATUS_OK;
}
