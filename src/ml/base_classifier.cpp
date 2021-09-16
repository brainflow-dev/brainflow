#include "base_classifier.h"
#include "brainflow_constants.h"
#include "spdlog/sinks/null_sink.h"


#define LOGGER_NAME "ml_logger"

#ifdef __ANDROID__
#include "spdlog/sinks/android_sink.h"
std::shared_ptr<spdlog::logger> BaseClassifier::ml_logger =
    spdlog::android_logger (LOGGER_NAME, "ml_ndk_logger");
#else
std::shared_ptr<spdlog::logger> BaseClassifier::ml_logger = spdlog::stderr_logger_mt (LOGGER_NAME);
#endif

int BaseClassifier::set_log_level (int level)
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
        BaseClassifier::ml_logger->set_level (spdlog::level::level_enum (log_level));
        BaseClassifier::ml_logger->flush_on (spdlog::level::level_enum (log_level));
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BaseClassifier::set_log_file (const char *log_file)
{
#ifdef __ANDROID__
    BaseClassifier::ml_logger->error ("For Android set_log_file is unavailable");
    return (int)BrainFlowExitCodes::GENERAL_ERROR;
#else
    try
    {
        spdlog::level::level_enum level = BaseClassifier::ml_logger->level ();
        BaseClassifier::ml_logger = spdlog::create<spdlog::sinks::null_sink_st> (
            "null_logger"); // to dont set logger to nullptr and avoid race condition
        spdlog::drop (LOGGER_NAME);
        BaseClassifier::ml_logger = spdlog::basic_logger_mt (LOGGER_NAME, log_file);
        BaseClassifier::ml_logger->set_level (level);
        BaseClassifier::ml_logger->flush_on (level);
        spdlog::drop ("null_logger");
    }
    catch (...)
    {
        return (int)BrainFlowExitCodes::GENERAL_ERROR;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
#endif
}
