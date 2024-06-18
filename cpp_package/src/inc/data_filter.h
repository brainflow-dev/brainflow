#pragma once

#include <complex>
#include <tuple>
#include <utility>
#include <vector>
// include it here to allow user include only this single file
#include "brainflow_array.h"
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
    /// set log level
    static void set_log_level (int log_level);
    /// set log file
    static void set_log_file (std::string log_file);
    /// write user defined string to BrainFlow logger
    static void log_message (int log_level, const char *format, ...);

    /// perform low pass filter in-place
    static void perform_lowpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    /// perform high pass filter in-place
    static void perform_highpass (double *data, int data_len, int sampling_rate, double cutoff,
        int order, int filter_type, double ripple);
    /// perform bandpass filter in-place
    static void perform_bandpass (double *data, int data_len, int sampling_rate, double start_freq,
        double stop_freq, int order, int filter_type, double ripple);
    /// perform bandstop filter in-place
    static void perform_bandstop (double *data, int data_len, int sampling_rate, double start_freq,
        double stop_freq, int order, int filter_type, double ripple);
    /// apply notch filter to remove env noise
    static void remove_environmental_noise (
        double *data, int data_len, int sampling_rate, int noise_type);
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
     * @param wavelet use WaveletTypes enum
     * @param decomposition_level level of decomposition in wavelet transform
     * @param extension use WaveletExtensionTypes enum
     * @return std::pair of wavelet coeffs array in format [A(J) D(J) D(J-1) ..... D(1)] where J is
     *              decomposition level A - app coeffs, D - detailed coeffs, and array of lengths for each block
     *              in wavelet coeffs array, length of this array is decomposition_level + 1
     */
    static std::pair<double *, int *> perform_wavelet_transform (
        double *data, int data_len, int wavelet, int decomposition_level, int extension_type = (int)WaveletExtensionTypes::SYMMETRIC);
    // clang-format on
    /// performs inverse wavelet transform
    static double *perform_inverse_wavelet_transform (std::pair<double *, int *> wavelet_output,
        int original_data_len, int wavelet, int decomposition_level,
        int extension_type = (int)WaveletExtensionTypes::SYMMETRIC);
    // clanf-format off
    /**
     * perform wavelet denoising
     * @param data input array, any size
     * @param data_len length of input array
     * @param wavelet use WaveletTypes enum
     * @param decomposition_level level of decomposition in wavelet transform
     * @param wavelet_denoising use WaveletDenoisingTypes enum
     * @param threshold use ThresholdTypes enum
     * @param extension use WaveletExtensionTypes enum
     * @param noise_level use NoiseEstimationLevelTypes enum
     */
    // clang-format on
    static void perform_wavelet_denoising (double *data, int data_len, int wavelet,
        int decomposition_level, int wavelet_denoising = (int)WaveletDenoisingTypes::SURESHRINK,
        int threshold = (int)ThresholdTypes::HARD,
        int extenstion_type = (int)WaveletExtensionTypes::SYMMETRIC,
        int noise_level = (int)NoiseEstimationLevelTypes::FIRST_LEVEL);
    /// restore data from selected detailed coeffs
    static void restore_data_from_wavelet_detailed_coeffs (double *data, int data_len, int wavelet,
        int decomposition_level, int level_to_restore, double *output);
    /// z score peak detection, more info https://stackoverflow.com/a/22640362
    static void detect_peaks_z_score (
        double *data, int data_len, int lag, double threshold, double influence, double *output);
    // clang-format off
    /**
    * calculate filters and the corresponding eigenvalues using the Common Spatial Patterns
    * @param data [n_epochs x n_channels x n_times]-shaped 3D array of data for two classes
    * @param labels n_epochs-length 1D array of zeros and ones that assigns class labels for each epoch. Zero corresponds to the first class
    * @param n_epochs the total number of epochs
    * @param n_channels the number of EEG channels
    * @param n_times the number of samples (observations) for a single epoch for a single channel 
    * @return pair of two arrays. The first [n_channel x n_channel]-shaped 2D array represents filters. The second n-channel length 1D array represents eigenvalues
    */
    static std::pair<BrainFlowArray<double, 2>, BrainFlowArray<double, 1>> get_csp (
        const BrainFlowArray<double, 3> &data, const BrainFlowArray<double,1> &labels);
    // clang-format on
    /// perform data windowing
    static double *get_window (int window_function, int window_len);
    /**
     * perform direct fft
     * @param data input array
     * @param data_len must be even
     * @param window window function
     * @param fft_len output fft len(data_len / 2 + 1)
     * @return complex array with size data_len / 2 + 1, it holds only positive im values
     */
    static std::complex<double> *perform_fft (double *data, int data_len, int window, int *fft_len);
    /**
     * perform inverse fft
     * @param fft_data complex array from perform_fft
     * @param fft_len len of original array, must be even
     * @param data_len output array len
     * @return restored data
     */
    static double *perform_ifft (std::complex<double> *fft_data, int fft_len, int *data_len);
    /**
     * calculate nearest power of 2
     * @param value input value
     * @return nearest power of 2
     */
    static int get_nearest_power_of_two (int value);
    /**
     * calculate PSD
     * @param data input array
     * @param data_len must be even
     * @param sampling_rate sampling rate
     * @param window window function
     * @param psd_len output len (data_len / 2 + 1)
     * @return pair of amplitude and freq arrays of size data_len / 2 + 1
     */
    static std::pair<double *, double *> get_psd (
        double *data, int data_len, int sampling_rate, int window, int *psd_len);
    /**
     * subtract trend from data
     * @param data input array
     * @param data_len
     * @param psd_len output len (data_len / 2 + 1)
     * @param detrend_operation use DetrendOperations enum
     */
    static void detrend (double *data, int data_len, int detrend_operation);
    static std::pair<double *, double *> get_psd_welch (double *data, int data_len, int nfft,
        int overlap, int sampling_rate, int window, int *psd_len);
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
     * @param channels vector of rows - eeg channels which should be used
     * @param sampling_rate sampling rate
     * @param apply_filters set to true to apply filters before band power calculations
     * @return pair of double arrays of size 5, first of them - avg band powers, second stddev
     */
    static std::pair<double *, double *> get_avg_band_powers (const BrainFlowArray<double, 2> &data,
        std::vector<int> channels, int sampling_rate, bool apply_filters);
    /**
     * calculate avg and stddev of BandPowers across all channels
     * @param data input 2d array
     * @param bands input bands
     * @param cols number of cols in 2d array - number of datapoints
     * @param channels vector of rows - eeg channels which should be used
     * @param sampling_rate sampling rate
     * @param apply_filters set to true to apply filters before band power calculations
     * @return pair of float arrays with the same size as bands argument
     */
    static std::pair<double *, double *> get_custom_band_powers (
        const BrainFlowArray<double, 2> &data, std::vector<std::pair<double, double>> bands,
        std::vector<int> channels, int sampling_rate, bool apply_filters);
    /**
     * calculate oxygen level
     * @param ppg_ir input 1d array
     * @param ppg_red input 1d array
     * @param data_len size of array
     * @param sampling_rate sampling rate
     * @return oxygen level
     */
    static double get_oxygen_level (double *ppg_ir, double *ppg_red, int data_len,
        int sampling_rate, double coef1 = 1.5958422, double coef2 = -34.6596622,
        double coef3 = 112.6898759);
    /**
     * calculate heart rate
     * @param ppg_ir input 1d array
     * @param ppg_red input 1d array
     * @param data_len size of array
     * @param sampling_rate sampling rate
     * @param fft_size recommended 8192
     * @return heart rate
     */
    static double get_heart_rate (
        double *ppg_ir, double *ppg_red, int data_len, int sampling_rate, int fft_size);
    /// write file, in file data will be transposed
    static void write_file (
        const BrainFlowArray<double, 2> &data, std::string file_name, std::string file_mode);
    /// read data from file, data will be transposed to original format
    static BrainFlowArray<double, 2> read_file (std::string file_name);
    /// calc stddev
    static double calc_stddev (double *data, int start_pos, int end_pos);
    /// calc railed percentage
    static double get_railed_percentage (double *data, int data_len, int gain);
    /**
     * calculate ICA
     * @param data input 2d array, rows are samples
     * @param num_components number of components to find
     * @param channels rows to use
     * @return unmixed signal
     */
    static std::tuple<BrainFlowArray<double, 2>, BrainFlowArray<double, 2>,
        BrainFlowArray<double, 2>, BrainFlowArray<double, 2>>
    perform_ica (
        const BrainFlowArray<double, 2> &data, int num_components, std::vector<int> channels);
    /**
     * calculate ICA
     * @param data input 2d array, rows are samples
     * @param num_components number of components to find
     * @return unmixed signal
     */
    static std::tuple<BrainFlowArray<double, 2>, BrainFlowArray<double, 2>,
        BrainFlowArray<double, 2>, BrainFlowArray<double, 2>>
    perform_ica (const BrainFlowArray<double, 2> &data, int num_components);


    /// get brainflow version
    static std::string get_version ();
};
