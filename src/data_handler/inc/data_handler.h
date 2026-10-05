#pragma once

#include "shared_export.h"

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
        int sampling_rate, double start_freq, double stop_freq, int order, int filter_type,
        double ripple);
    SHARED_EXPORT int CALLING_CONVENTION perform_bandstop (double *data, int data_len,
        int sampling_rate, double start_freq, double stop_width, int order, int filter_type,
        double ripple);
    SHARED_EXPORT int CALLING_CONVENTION remove_environmental_noise (
        double *data, int data_len, int sampling_rate, int noise_type);
    SHARED_EXPORT int CALLING_CONVENTION perform_rolling_filter (
        double *data, int data_len, int period, int agg_operation);
    SHARED_EXPORT int CALLING_CONVENTION perform_downsampling (
        double *data, int data_len, int period, int agg_operation, double *output_data);
    // FIR decimation: floor(data_len/factor) outputs, first sample aligned to input[0].
    SHARED_EXPORT int CALLING_CONVENTION perform_decimation (
        const double *data, int data_len, int factor, double *output_data);
    // Streaming causal filter kinds: lowpass=0, highpass=1, bandpass=2, bandstop=3.
    SHARED_EXPORT int CALLING_CONVENTION get_filter_settling_samples (int kind, int sampling_rate,
        double low_cutoff, double high_cutoff, int order, int filter_type, double ripple,
        int *edge_samples);
    SHARED_EXPORT int CALLING_CONVENTION create_streaming_filter (int kind, int sampling_rate,
        double low_cutoff, double high_cutoff, int order, int filter_type, double ripple,
        int *handle);
    SHARED_EXPORT int CALLING_CONVENTION perform_streaming_filter (
        int handle, double *data, int data_len);
    SHARED_EXPORT int CALLING_CONVENTION reset_streaming_filter (int handle);
    SHARED_EXPORT int CALLING_CONVENTION release_streaming_filter (int handle);
    SHARED_EXPORT int CALLING_CONVENTION perform_wavelet_transform (double *data, int data_len,
        int wavelet, int decomposition_level, int extension, double *output_data,
        int *decomposition_lengths);
    SHARED_EXPORT int CALLING_CONVENTION perform_inverse_wavelet_transform (double *wavelet_coeffs,
        int original_data_len, int wavelet, int decomposition_level, int extension,
        int *decomposition_lengths, double *output_data);
    SHARED_EXPORT int CALLING_CONVENTION perform_inverse_wavelet_transform_checked (double *coeffs,
        int coeff_count, int original_data_len, int wavelet, int level, int extension, int *lengths,
        int lengths_count, double *output, int output_count);
    SHARED_EXPORT int CALLING_CONVENTION perform_wavelet_denoising (double *data, int data_len,
        int wavelet, int decomposition_level, int wavelet_denoising, int threshold,
        int extenstion_type, int noise_level);
    SHARED_EXPORT int CALLING_CONVENTION get_csp (const double *data, const double *labels,
        int n_epochs, int n_channels, int n_times, double *output_w, double *output_d);
    SHARED_EXPORT int CALLING_CONVENTION get_window (
        int window_function, int window_len, double *output_window);
    SHARED_EXPORT int CALLING_CONVENTION perform_fft (
        double *data, int data_len, int window_function, double *output_re, double *output_im);
    SHARED_EXPORT int CALLING_CONVENTION perform_ifft (
        double *input_re, double *input_im, int data_len, double *restored_data);
    SHARED_EXPORT int CALLING_CONVENTION get_nearest_power_of_two (int value, int *output);
    SHARED_EXPORT int CALLING_CONVENTION get_psd (double *data, int data_len, int sampling_rate,
        int window_function, double *output_ampl, double *output_freq);
    SHARED_EXPORT int CALLING_CONVENTION detrend (
        double *data, int data_len, int detrend_operation);
    SHARED_EXPORT int CALLING_CONVENTION calc_stddev (
        double *data, int start_pos, int end_pos, double *output);
    SHARED_EXPORT int CALLING_CONVENTION get_psd_welch (double *data, int data_len, int nfft,
        int overlap, int sampling_rate, int window_function, double *output_ampl,
        double *output_freq);
    SHARED_EXPORT int CALLING_CONVENTION get_band_power (double *ampl, double *freq, int data_len,
        double freq_start, double freq_end, double *band_power);
    SHARED_EXPORT int CALLING_CONVENTION get_custom_band_powers (double *raw_data, int rows,
        int cols, double *start_freqs, double *stop_freqs, int num_bands, int sampling_rate,
        int apply_filters, double *avg_band_powers, double *stddev_band_powers);
    // nfft=0 selects the default; cutoff=0 disables that passband edge.
    // mains: -1=automatic, 0=none, 1=50Hz, 2=60Hz, 3=both.
    SHARED_EXPORT int CALLING_CONVENTION get_band_power_settings (int sampling_rate,
        int apply_filters, int nfft, double low_cutoff, double high_cutoff, int mains,
        int *effective_nfft, int *edge_samples);
    SHARED_EXPORT int CALLING_CONVENTION get_custom_band_powers_with_options (double *raw_data,
        int rows, int cols, double *start_freqs, double *stop_freqs, int num_bands,
        int sampling_rate, int apply_filters, int nfft, double low_cutoff, double high_cutoff,
        int mains, int detrend_operation, double *avg_band_powers, double *stddev_band_powers);
    SHARED_EXPORT int CALLING_CONVENTION get_railed_percentage (
        double *raw_data, int data_len, int gain, double *output);
    SHARED_EXPORT int CALLING_CONVENTION get_clipping_percentage (
        const double *data, int data_len, double lower_bound, double upper_bound, double *output);
    SHARED_EXPORT int CALLING_CONVENTION get_flatline_percentage (
        const double *data, int data_len, double tolerance, double *output);
    SHARED_EXPORT int CALLING_CONVENTION get_oxygen_level (double *ppg_ir, double *ppg_red,
        int data_size, int sampling_rate, double callib_coef1, double callib_coef2,
        double callib_coef3, double *oxygen_level);
    SHARED_EXPORT int CALLING_CONVENTION get_heart_rate (double *ppg_ir, double *ppg_red,
        int data_size, int sampling_rate, int fft_size, double *rate);
    SHARED_EXPORT int CALLING_CONVENTION restore_data_from_wavelet_detailed_coeffs (double *data,
        int data_len, int wavelet, int decomposition_level, int level_to_restore, double *output);
    SHARED_EXPORT int CALLING_CONVENTION detect_peaks_z_score (
        double *data, int data_len, int lag, double threshold, double influence, double *output);
    SHARED_EXPORT int CALLING_CONVENTION perform_ica (double *data, int rows, int cols,
        int num_components, double *w_mat, double *k_mat, double *a_mat, double *s_mat);
    SHARED_EXPORT int CALLING_CONVENTION perform_ica_with_options (double *data, int rows, int cols,
        int num_components, double *w_mat, double *k_mat, double *a_mat, double *s_mat,
        int max_iterations, double tolerance, int seed);
    SHARED_EXPORT int CALLING_CONVENTION get_activity_index (const double *accel_x,
        const double *accel_y, const double *accel_z, int data_len, int sampling_rate, int period,
        double noise_var_x, double noise_var_y, double noise_var_z, double *activity_index);

    // logging methods
    SHARED_EXPORT int CALLING_CONVENTION set_log_level_data_handler (int log_level);
    SHARED_EXPORT int CALLING_CONVENTION set_log_file_data_handler (const char *log_file);
    SHARED_EXPORT int CALLING_CONVENTION log_message_data_handler (int log_level, char *message);

    // file operations
    SHARED_EXPORT int CALLING_CONVENTION write_file (const double *data, int num_rows, int num_cols,
        const char *file_name, const char *file_mode);
    SHARED_EXPORT int CALLING_CONVENTION read_file (
        double *data, int *num_rows, int *num_cols, const char *file_name, int num_elements);
    SHARED_EXPORT int CALLING_CONVENTION get_num_elements_in_file (
        const char *file_name, int *num_elements); // its an internal method for bindings its not
                                                   // available via high level api

    // platform types and methods
    SHARED_EXPORT int CALLING_CONVENTION get_version_data_handler (
        char *version, int *num_chars, int max_chars);
#ifdef __cplusplus
}
#endif
