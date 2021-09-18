#pragma once

#include "shared_export.h"

#ifdef __cplusplus

#include <memory>

class brainflow_filter
{
public:
    brainflow_filter () {};
    virtual ~brainflow_filter () = 0;
    brainflow_filter (brainflow_filter &&) = delete;
    brainflow_filter (const brainflow_filter &) = delete;
    brainflow_filter &operator= (brainflow_filter &&) = delete;
    brainflow_filter &operator= (const brainflow_filter &) = delete;

    virtual int process (double *data, int data_len) = 0;
};

SHARED_EXPORT std::unique_ptr<brainflow_filter> create_lowpass_filter (
    int sampling_rate, double cutoff, int order, int filter_type, double ripple);
SHARED_EXPORT std::unique_ptr<brainflow_filter> create_highpass_filter (
    int sampling_rate, double cutoff, int order, int filter_type, double ripple);
SHARED_EXPORT std::unique_ptr<brainflow_filter> create_bandpass_filter (int sampling_rate,
    double center_freq, double band_width, int order, int filter_type, double ripple);
SHARED_EXPORT std::unique_ptr<brainflow_filter> create_bandstop_filter (int sampling_rate,
    double center_freq, double band_width, int order, int filter_type, double ripple);
SHARED_EXPORT std::unique_ptr<brainflow_filter> create_environmental_noise_filter (
    int sampling_rate, int noise_type);
SHARED_EXPORT std::unique_ptr<brainflow_filter> create_rolling_filter (
    int period, int agg_operation);
SHARED_EXPORT std::unique_ptr<brainflow_filter> create_downsampling_filter (
    int period, int agg_operation);

#endif
