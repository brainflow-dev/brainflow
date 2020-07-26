#include <map>
#include <mutex>

#include "brainflow_constants.h"
#include "metrics_calculator.h"
#include "metrics_handler.h"


std::map<int, std::shared_ptr<MetricsHandler>> metrics_handlers;
std::mutex metrics_mutex;


int prepare (int calculator)
{
    std::lock_guard<std::mutex> lock (metrics_mutex);

    if (metrics_handlers.find (calculator) != metrics_handler.end ())
    {
        return (int)BrainFlowExitCodes::ANOTHER_SESSION_IS_PREPARED;
    }

    std::shared_ptr<MetricsHandler> handler = NULL;
    switch (static_cast<EEGMetricsCalculators> (calculator))
    {
        case EEGMetricsCalculators::MANUAL:
            handler = std::shared_ptr<MetricsHandler> (new ManualEEGHandler ());
            break;
        case EEGMetricsCalculators::SVM:
            handler = std::shared_ptr<MetricsHandler> (new SVMEEGHandler ());
            break;
        default:
            return (int)BrainFlowExitCodes::INVALID_ARGUMENTS_ERROR;
    }
    res = handler->prepare ();
    if (res != (int)BrainFlowExitCodes::STATUS_OK)
    {
        handler = NULL;
    }
    else
    {
        metrics_handlers[key] = handler;
    }

    return res;
}

int calc_relaxation (
    double *avg_bandpowers, double *stddev_bandpowers, int data_len, double *output, int calculator)
{
    std::lock_guard<std::mutex> lock (metrics_mutex);

    auto handler_it = metrics_handlers.find (calculator);
    if (handler_it == metrics_handlers.end ())
    {
        return (int)BrainFlowExitCodes::SESSION_IS_NOT_PREPARED;
    }
    return metrics_it->calc_relaxation (
        avg_bandpowers, stddev_bandpowers, data_len, classifier, output);
}

int calc_concentration (
    double *avg_bandpowers, double *stddev_bandpowers, int data_len, int classifier, double *output)
{
    std::lock_guard<std::mutex> lock (metrics_mutex);

    auto handler_it = metrics_handlers.find (calculator);
    if (handler_it == metrics_handlers.end ())
    {
        return (int)BrainFlowExitCodes::SESSION_IS_NOT_PREPARED;
    }
    return metrics_it->calc_concentration (
        avg_bandpowers, stddev_bandpowers, data_len, classifier, output);
}

int release ()
{
    std::lock_guard<std::mutex> lock (metrics_mutex);

    if (metrics_handler != NULL)
    {
        delete metrics_handler;
        metrics_handler = NULL;
    }
    return (int)BrainFlowExitCodes::STATUS_OK;
}
