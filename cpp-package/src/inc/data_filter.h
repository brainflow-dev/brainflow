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
    /**
     * perform direct fft
     * @param data input array
     * @param data_len must be power of 2
     * @return complex array with size data_len / 2 + 1, it holds only positive im values
     */
    static std::complex<double> *perform_fft (double *data, int data_len);
    /**
     * perform inverse fft
     * @param data complex array from perform_fft
     * @param data_len len of original array, must be power of 2
     * @return restored data
     */
    static double *perform_ifft (std::complex<double> *data, int data_len);


    /// write file, in file data will be transposed
    static void write_file (
        double **data, int num_rows, int num_cols, char *file_name, char *file_mode);
    /// read data from file, data will be transposed to original format
    static double **read_file (int *num_rows, int *num_cols, char *file_name);

private:
    static void reshape_data_to_2d (
        int num_rows, int num_cols, double *linear_buffer, double **output_buf);
    static void reshape_data_to_1d (int num_rows, int num_cols, double **buf, double *output_buf);
};
