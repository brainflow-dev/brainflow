#pragma once

#ifdef _WIN32
#define SHARED_EXPORT __declspec(dllexport)
#else
#define SHARED_EXPORT
#endif

#include "brainflow_constants.h"

#define MAX_FILTER_ORDER 8 // define it here to show in the docs


typedef enum
{
    BUTTERWORTH = 0,
    CHEBYSHEV_TYPE_1 = 1,
    BESSEL = 2
} FilterTypes;

// its another kind of filters and will be used in downsampling too dont add it to FilterTypes
typedef enum
{
    MEAN = 0,
    MEDIAN = 1,
    EACH = 2
} AggOperations;


#ifdef __cplusplus
extern "C"
{
#endif
    // signal processing methods
    // ripple param uses only for chebyshev filter
    SHARED_EXPORT int perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    SHARED_EXPORT int perform_highpass (double *data, int data_len, int sampling_rate,
        double cutoff, int order, int filter_type, double ripple);
    SHARED_EXPORT int perform_bandpass (double *data, int data_len, int sampling_rate,
        double center_freq, double band_width, int order, int filter_type, double ripple);
    SHARED_EXPORT int perform_bandstop (double *data, int data_len, int sampling_rate,
        double center_freq, double band_width, int order, int filter_type, double ripple);
    SHARED_EXPORT int perform_rolling_filter (
        double *data, int data_len, int period, int agg_operation);

    // file operations
    SHARED_EXPORT int write_file (
        double *data, int num_rows, int num_cols, char *file_name, char *file_mode);
    SHARED_EXPORT int read_file (
        double *data, int *num_rows, int *num_cols, char *file_name, int num_elements);
    SHARED_EXPORT int get_num_elements_in_file (
        char *file_name, int *num_elements); // its an internal method for bindings its not
                                             // available via high level api
#ifdef __cplusplus
}
#endif
