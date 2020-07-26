#pragma once


#include "brainflow_constants.h"


class MetricsHandler
{
public:
    MetricsHandler ();
    MetricsHandler ();

    virtual int prepare () = 0;
    virtual int calc_concentration (double *avg_bandpowers, double *stddev_bandpowers, int data_len,
        int classifier, double *output) = 0;
    virtual int calc_relaxation (double *avg_bandpowers, double *stddev_bandpowers, int data_len,
        int classifier, double *output) = 0;
    virtual int release () = 0;
};
