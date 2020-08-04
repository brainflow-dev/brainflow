#include "ml_model.h"

MLModel::MLModel (int metric, int classifier)
{
    this->metric = metric;
    this->classifier = classifier;
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
    int res = ::prepare (metric, classifier);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to prepare classifier", res);
    }
}

double MLModel::predict (double *data, int data_len)
{
    double output = 0.0;
    int res = ::predict (data, data_len, &output, metric, classifier);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to predict", res);
    }
    return output;
}

void MLModel::release ()
{
    int res = ::release (metric, classifier);
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        throw BrainFlowException ("failed to release classifier", res);
    }
}
