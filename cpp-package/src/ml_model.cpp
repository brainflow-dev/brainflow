#include "ml_model.h"
#include "brainflow_constants.h"
#include "json.hpp"
#include "ml_module.h"

using json = nlohmann::json;


std::string params_to_string (struct BrainFlowModelParams params)
{
    json j;
    j["metric"] = params.metric;
    j["classifier"] = params.classifier;
    j["file"] = params.file;
    j["other_info"] = params.other_info;
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

double MLModel::predict (double *data, int data_len)
{
    double output = 0.0;
    int res = ::predict (data, data_len, &output, serialized_params.c_str ());
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to predict", res);
    }
    return output;
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
