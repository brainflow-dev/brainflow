#pragma once

#include "brainflow_constants.h"
#include "shared_export.h"

#define MAX_FILTER_ORDER 8 // define it here to show in the docs


typedef enum
{
    BUTTERWORTH = 0,
    CHEBYSHEV_TYPE_1 = 1,
    BESSEL = 2
} FilterTypes;

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
    // ripple param is used only for chebyshev filter
    SHARED_EXPORT int CALLING_CONVENTION perform_lowpass (double *data, int data_len,
        int sampling_rate, double cutoff, int order, int filter_type, double ripple);
    SHARED_EXPORT int CALLING_CONVENTION perform_highpass (double *data, int data_len,
        int sampling_rate, double cutoff, int order, int filter_type, double ripple);
    SHARED_EXPORT int CALLING_CONVENTION perform_bandpass (double *data, int data_len,
        int sampling_rate, double center_freq, double band_width, int order, int filter_type,
        double ripple);
    SHARED_EXPORT int CALLING_CONVENTION perform_bandstop (double *data, int data_len,
        int sampling_rate, double center_freq, double band_width, int order, int filter_type,
        double ripple);

    SHARED_EXPORT int CALLING_CONVENTION perform_rolling_filter (
        double *data, int data_len, int period, int agg_operation);

    SHARED_EXPORT int CALLING_CONVENTION perform_downsampling (
        double *data, int data_len, int period, int agg_operation, double *output_data);

    SHARED_EXPORT int CALLING_CONVENTION perform_wavelet_transform (double *data, int data_len,
        char *wavelet, int decomposition_level, double *output_data, int *decomposition_lengths);
    SHARED_EXPORT int CALLING_CONVENTION perform_inverse_wavelet_transform (double *wavelet_coeffs,
        int original_data_len, char *wavelet, int decomposition_level, int *decomposition_lengths,
        double *output_data);
    SHARED_EXPORT int CALLING_CONVENTION perform_wavelet_denoising (
        double *data, int data_len, char *wavelet, int decomposition_level);
    SHARED_EXPORT int CALLING_CONVENTION perform_fft (
        double *data, int data_len, double *output_re, double *output_im);
    SHARED_EXPORT int CALLING_CONVENTION perform_ifft (
        double *input_re, double *input_im, int data_len, double *restored_data);


    // file operations
    SHARED_EXPORT int CALLING_CONVENTION write_file (
        double *data, int num_rows, int num_cols, char *file_name, char *file_mode);
    SHARED_EXPORT int CALLING_CONVENTION read_file (
        double *data, int *num_rows, int *num_cols, char *file_name, int num_elements);
    SHARED_EXPORT int CALLING_CONVENTION get_num_elements_in_file (
        char *file_name, int *num_elements); // its an internal method for bindings its not
                                             // available via high level api
#ifdef __cplusplus
}
#endif
