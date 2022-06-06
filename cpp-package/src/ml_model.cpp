#include <cstdarg>
#include <stdlib.h>

#include "brainflow_constants.h"
#include "json.hpp"
#include "ml_model.h"
#include "ml_module.h"

using json = nlohmann::json;


void MLModel::release_all ()
{
    int res = ::release_all ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to release classifiers", res);
    }
}

std::string params_to_string (struct BrainFlowModelParams params)
{
    json j;
    j["metric"] = params.metric;
    j["classifier"] = params.classifier;
    j["file"] = params.file;
    j["other_info"] = params.other_info;
    j["output_name"] = params.output_name;
    j["max_array_size"] = params.max_array_size;
    std::string post_str = j.dump ();
    return post_str;
}

MLModel::MLModel (struct BrainFlowModelParams model_params) : params (model_params)
{
    serialized_params = params_to_string (model_params);
}

void MLModel::prepare ()
{
    int res = ::prepare (serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to prepare classifier", res);
    }
}

std::vector<double> MLModel::predict (double *data, int data_len)
{
    double *output = new double[params.max_array_size];
    int size = 0;
    int res = ::predict (data, data_len, output, &size, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        delete[] output;
        throw BrainFlowException ("failed to predict", res);
    }
    std::vector<double> result (size);
    std::copy (output, output + size, result.begin ());
    delete[] output;
    return result;
}

void MLModel::release ()
{
    int res = ::release (serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to release classifier", res);
    }
}

/////////////////////////////////////////
//////////// logging methods ////////////
/////////////////////////////////////////

void MLModel::set_log_file (std::string log_file)
{
    int res = set_log_file_ml_module (log_file.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to set log file", res);
    }
}

void MLModel::set_log_level (int log_level)
{
    int res = set_log_level_ml_module (log_level);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to set log level", res);
    }
}

void MLModel::log_message (int log_level, const char *format, ...)
{
    char buffer[1024];
    va_list ap;
    va_start (ap, format);
    vsnprintf (buffer, 1024, format, ap);
    va_end (ap);

    int res = log_message_ml_module (log_level, buffer);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to write log message", res);
    }
}

void MLModel::enable_ml_logger ()
{
    MLModel::set_log_level ((int)LogLevels::LEVEL_INFO);
}

void MLModel::disable_ml_logger ()
{
    MLModel::set_log_level ((int)LogLevels::LEVEL_OFF);
}

void MLModel::enable_dev_ml_logger ()
{
    MLModel::set_log_level ((int)LogLevels::LEVEL_TRACE);
}

std::string MLModel::get_version ()
{
    char version[64];
    int string_len = 0;
    int res = ::get_version_ml_module (version, &string_len, 64);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to get board info", res);
    }
    std::string verion_str (version, string_len);

    return verion_str;
}
