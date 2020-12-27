#pragma once

#include <complex>
#include <utility>
// include it here to allow user include only this single file
#include "brainflow_constants.h"
#include "brainflow_exception.h"
#include "data_handler.h"

/// DataFilter class to perform signal processing
class DataFilter
{
public:
    /// enable Data logger with LEVEL_INFO
    static void enable_data_logger ();
    /// disable Data loggers
    static void disable_data_logger ();
    /// enable Data logger with LEVEL_TRACE
    static void enable_dev_data_logger ();

    static void set_log_file (char *log_file);
    /// perform low pass filter in-place
    static void perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    /// perform high pass filter in-place
    static void perform_highpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    /// perform bandpass filter in-place
    static void perform_bandpass (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);
    /// perform bandstop filter in-place
    static void perform_bandstop (double *data, int data_len, int sampling_rate, double center_freq,
        double band_width, int order, int filter_type, double ripple);
    /// perform moving average or moving median filter in-place
    static void perform_rolling_filter (double *data, int data_len, int period, int agg_operation);
    /// perform data downsampling, it just aggregates several data points
    static double *perform_downsampling (
        double *data, int data_len, int period, int agg_operation, int *filtered_size);
    // clang-format off
    /**
     * perform wavelet transform
     * @param data input array, any size
     * @param data_len length of input array
     * @param wavelet supported vals:
     *              db1..db15,haar,sym2..sym10,coif1..coif5,bior1.1,bior1.3,bior1.5,bior2.2,bior2.4,bior2.6,bior2.8,bior3.1,bior3.3,bior3.5
     *              ,bior3.7,bior3.9,bior4.4,bior5.5,bior6.8
     * @param decomposition_level level of decomposition in wavelet transform
     * @return std::pair of wavelet coeffs array in format [A(J) D(J) D(J-1) ..... D(1)] where J is
     *              decomposition level A - app coeffs, D - detailed coeffs, and array of lengths for each block
     *              in wavelet coeffs array, length of this array is decomposition_level + 1
     */
    static std::pair<double *, int *> perform_wavelet_transform (
        double *data, int data_len, char *wavelet, int decomposition_level);
    // clang-format on
    /// performs inverse wavelet transform
    static double *perform_inverse_wavelet_transform (std::pair<double *, int *> wavelet_output,
        int original_data_len, char *wavelet, int decomposition_level);
    /// perform wavelet denoising
    static void perform_wavelet_denoising (
        double *data, int data_len, char *wavelet, int decomposition_level);
    /// perform data windowing
    static double *get_window (int window_function, int window_len);
    /**
     * perform direct fft
     * @param data input array
     * @param data_len must be power of 2
     * @param window window function
     * @return complex array with size data_len / 2 + 1, it holds only positive im values
     */
    static std::complex<double> *perform_fft (double *data, int data_len, int window);
    /**
     * perform inverse fft
     * @param data complex array from perform_fft
     * @param data_len len of original array, must be power of 2
     * @return restored data
     */
    static double *perform_ifft (std::complex<double> *data, int data_len);
    /**
     * calculate nearest power of 2
     * @param value input value
     * @return nearest power of 2
     */
    static int get_nearest_power_of_two (int value);
    /**
     * calculate PSD
     * @param data input array
     * @param data_len must be power of 2
     * @param sampling_rate sampling rate
     * @param window window function
     * @return pair of amplitude and freq arrays of size data_len / 2 + 1
     */
    static std::pair<double *, double *> get_psd (
        double *data, int data_len, int sampling_rate, int window);
    /**
     * subtract trend from data
     * @param data input array
     * @param data_len
     * @param detrend_operation use DetrendOperations enum
     */
    static void detrend (double *data, int data_len, int detrend_operation);
    static std::pair<double *, double *> get_psd_welch (
        double *data, int data_len, int nfft, int overlap, int sampling_rate, int window);
    /**
     * calculate band power
     * @param psd psd calculated using get_psd
     * @param data_len len of ampl and freq arrays: N / 2 + 1 where N is FFT size
     * @param freq_start lowest frequency
     * @param freq_end highest frequency
     * @return band power
     */
    static double get_band_power (
        std::pair<double *, double *> psd, int data_len, double freq_start, double freq_end);
    /**
     * calculate avg and stddev of BandPowers across all channels
     * @param data input 2d array
     * @param cols number of cols in 2d array - number of datapoints
     * @param channels array of rows - eeg channels which should be used
     * @param channels_len - len of channels array
     * @param sampling_rate sampling rate
     * @param apply_filters set to true to apply filters before band power calculations
     * @return pair of double arrays of size 5, first of them - avg band powers, second stddev
     */
    static std::pair<double *, double *> get_avg_band_powers (double **data, int cols,
        int *channels, int channels_len, int sampling_rate, bool apply_filters);

    /// write file, in file data will be transposed
    static void write_file (
        double **data, int num_rows, int num_cols, char *file_name, char *file_mode);
    /// read data from file, data will be transposed to original format
    static double **read_file (int *num_rows, int *num_cols, char *file_name);

private:
    static void set_log_level (int log_level);
    static void reshape_data_to_2d (
        int num_rows, int num_cols, double *linear_buffer, double **output_buf);
    static void reshape_data_to_1d (int num_rows, int num_cols, double **buf, double *output_buf);
};
