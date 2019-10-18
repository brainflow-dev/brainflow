#pragma once

// include it here to allow user include only this single file
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "data_handler.h"

class DataFilter
{
public:
    static void perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    static void perform_highpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    static void perform_bandpass (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);
    static void perform_bandstop (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);
};
