#include <algorithm>
#include <cmath>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "concentration_regression_classifier.h"

#include "spdlog/sinks/null_sink.h"

#define LOGGER_NAME "brainflow_logger"

#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
std::shared_ptr<spdlog::logger> ConcentrationRegressionClassifier::ml_logger =
    spdlog::android_logger (LOGGER_NAME, "brainflow_ndk_logger");
#else
std::shared_ptr<spdlog::logger> ConcentrationRegressionClassifier::ml_logger = spdlog::stderr_logger_mt (LOGGER_NAME);
#endif

int ConcentrationRegressionClassifier::set_log_level (int level)
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
        ConcentrationRegressionClassifier::ml_logger->set_level (spdlog::level::level_enum (log_level));
        ConcentrationRegressionClassifier::ml_logger->flush_on (spdlog::level::level_enum (log_level));
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::set_log_file (char *log_file)
{
#ifdef __ANDROID__
    ConcentrationRegressionClassifier::ml_logger->error ("For Android set_log_file is unavailable");
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
#else
    try
    {
        spdlog::level::level_enum level = ConcentrationRegressionClassifier::ml_logger->level ();
        ConcentrationRegressionClassifier::ml_logger = spdlog::create<spdlog::sinks::null_sink_st> (
            "null_logger"); // to dont set logger to nullptr and avoid raice condition
        spdlog::drop (LOGGER_NAME);
        ConcentrationRegressionClassifier::ml_logger = spdlog::basic_logger_mt (LOGGER_NAME, log_file);
        ConcentrationRegressionClassifier::ml_logger->set_level (level);
        ConcentrationRegressionClassifier::ml_logger->flush_on (level);
        spdlog::drop ("null_logger");
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}

int ConcentrationRegressionClassifier::prepare ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::predict (double *data, int data_len, double *output)
{
    if ((data_len < 5) || (data == NULL) || (output == NULL))
    {
        safe_logger (spdlog::level::err, "Classifier has already been prepared.");
        return (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR;
    }
    double value = 0.0;
    // coefficients from focus_classifier.py, first 5 - avg band powers, last 5 stddevs
    const double coefficients[10] = {-4.00053368, -1.41176333, -15.76696787, 7.03090858,
        18.15572423, -3.69625477, 3.18110813, -8.27378987, 2.30053461, 2.94329616};
    double intercept = 4.00736793;
    // undocumented feature(not recommended): may work without stddev but with worse accuracy
    for (int i = 0; i < std::min (data_len, 10); i++)
    {
        value += coefficients[i] * data[i];
    }
    double concentration = 1.0 / (1.0 + exp (-1.0 * (intercept + value)));
    *output = concentration;
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int ConcentrationRegressionClassifier::release ()
{
    return (int)BrainFlowExitCodes::STATUS_OK;
}
