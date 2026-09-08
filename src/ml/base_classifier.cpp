#include <string>
#include <vector>

#include "base_classifier.h"
#include "brainflow_constants.h"
#include "json.hpp"
#include "spdlog/sinks/null_sink.h"

using json = nlohmann::json;

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

void BaseClassifier::parse_moving_average_params ()
{
    use_moving_average = false;
    moving_average_window = 0;

    if (params.other_info.empty ())
    {
        return;
    }

    try
    {
        json j = json::parse (params.other_info);
        if (!j.is_object ())
        {
            return;
        }

        if (j.contains ("moving_average"))
        {
            if (j["moving_average"].is_boolean ())
            {
                if (!j["moving_average"].get<bool> ())
                {
                    return;
                }
                use_moving_average = true;
                moving_average_window = DEFAULT_MOVING_AVERAGE_WINDOW;
            }
            else if (j["moving_average"].is_number_integer ())
            {
                int val = j["moving_average"].get<int> ();
                if (val <= 0)
                {
                    return;
                }
                use_moving_average = true;
                moving_average_window = val;
            }
        }

        if (j.contains ("window_len") && j["window_len"].is_number_integer ())
        {
            int val = j["window_len"].get<int> ();
            if (val > 0)
            {
                moving_average_window = val;
                use_moving_average = true;
            }
            else
            {
                use_moving_average = false;
                moving_average_window = 0;
            }
        }
    }
    catch (const std::exception &)
    {
        use_moving_average = false;
        moving_average_window = 0;
    }
}

void BaseClassifier::reset_moving_average ()
{
    window_data.clear ();
}

void BaseClassifier::apply_moving_average (double *output, int *output_len)
{
    if ((output == NULL) || (output_len == NULL) || (*output_len <= 0) || (moving_average_window <= 0))
    {
        return;
    }

    if (window_data.empty () || ((int)window_data.front ().size () != *output_len))
    {
        window_data.clear ();
    }

    window_data.push_back (std::vector<double> (output, output + *output_len));
    while ((int)window_data.size () > moving_average_window)
    {
        window_data.pop_front ();
    }

    if (window_data.empty ())
    {
        return;
    }

    for (int i = 0; i < *output_len; i++)
    {
        double sum = 0.0;
        for (size_t w = 0; w < window_data.size (); w++)
        {
            sum += window_data[w][i];
        }
        output[i] = sum / window_data.size ();
    }
}

int BaseClassifier::prepare ()
{
    parse_moving_average_params ();
    reset_moving_average ();
    return prepare_classifier ();
}

int BaseClassifier::predict (double *data, int data_len, double *output, int *output_len)
{
    int res = calculate (data, data_len, output, output_len);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        return res;
    }
    if (use_moving_average)
    {
        apply_moving_average (output, output_len);
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}

int BaseClassifier::release ()
{
    reset_moving_average ();
    return release_classifier ();
}
