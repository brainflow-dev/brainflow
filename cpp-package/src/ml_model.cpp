#include "ml_model.h"

#include "json.hpp"

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

MLModel::~MLModel ()
{
    try
    {
        release ();
    }
    catch (...)
    {
        // do nothing
    }
}

void MLModel::prepare ()
{
    int res = ::prepare (const_cast<char *> (serialized_params.c_str ()));
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to prepare classifier", res);
    }
}

double MLModel::predict (double *data, int data_len)
{
    double output = 0.0;
    int res = ::predict (data, data_len, &output, const_cast<char *> (serialized_params.c_str ()));
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to predict", res);
    }
    return output;
}

void MLModel::release ()
{
    int res = ::release (const_cast<char *> (serialized_params.c_str ()));
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to release classifier", res);
    }
}
