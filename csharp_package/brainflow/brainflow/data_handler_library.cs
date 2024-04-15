using System.Runtime.InteropServices;

namespace brainflow
{
    public enum FilterTypes
    {
        BUTTERWORTH = 0,
        CHEBYSHEV_TYPE_1 = 1,
        BESSEL = 2,
        BUTTERWORTH_ZERO_PHASE = 3,
        CHEBYSHEV_TYPE_1_ZERO_PHASE = 4,
        BESSEL_ZERO_PHASE = 5
    };

    public enum AggOperations
    {
        MEAN = 0,
        MEDIAN = 1,
        EACH = 2
    };

    public enum WindowOperations
    {
        NO_WINDOW = 0,
        HANNING = 1,
        HAMMING = 2,
        BLACKMAN_HARRIS = 3
    };

    public enum DetrendOperations
    {
        NO_DETREND = 0,
        CONSTANT = 1,
        LINEAR = 2
    };

    public enum NoiseTypes
    {
        FIFTY = 0,
        SIXTY = 1,
        FIFTY_AND_SIXTY = 2
    };


    public enum WaveletDenoisingTypes
    {
        VISUSHRINK = 0,
        SURESHRINK = 1
    };

    public enum ThresholdTypes
    {
        SOFT = 0,
        HARD = 1
    };

    public enum WaveletExtensionTypes
    {
        SYMMETRIC = 0,
        PERIODIC = 1
    };

    public enum NoiseEstimationLevelTypes
    {
        FIRST_LEVEL = 0,
        ALL_LEVELS = 1
    };

    public enum WaveletTypes
    {
        HAAR = 0,
        DB1 = 1,
        DB2 = 2,
        DB3 = 3,
        DB4 = 4,
        DB5 = 5,
        DB6 = 6,
        DB7 = 7,
        DB8 = 8,
        DB9 = 9,
        DB10 = 10,
        DB11 = 11,
        DB12 = 12,
        DB13 = 13,
        DB14 = 14,
        DB15 = 15,
        BIOR1_1 = 16,
        BIOR1_3 = 17,
        BIOR1_5 = 18,
        BIOR2_2 = 19,
        BIOR2_4 = 20,
        BIOR2_6 = 21,
        BIOR2_8 = 22,
        BIOR3_1 = 23,
        BIOR3_3 = 24,
        BIOR3_5 = 25,
        BIOR3_7 = 26,
        BIOR3_9 = 27,
        BIOR4_4 = 28,
        BIOR5_5 = 29,
        BIOR6_8 = 30,
        COIF1 = 31,
        COIF2 = 32,
        COIF3 = 33,
        COIF4 = 34,
        COIF5 = 35,
        SYM2 = 36,
        SYM3 = 37,
        SYM4 = 38,
        SYM5 = 39,
        SYM6 = 40,
        SYM7 = 41,
        SYM8 = 42,
        SYM9 = 43,
        SYM10 = 44
    };

    class DataHandlerLibrary64
    {
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_data_handler (string log_file);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int log_message_data_handler (int log_level, string message);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_data_handler (int log_level);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_rolling_filter (double[] data, int len, int period, int operation);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_downsampling (double[] data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_elements_in_file (string file_name, int[] num_elements);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_transform (double[] data, int data_len, int wavelet, int decomposition_level, int extension, double[] output_data, int[] decomposition_lengths);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, int wavelet, int decomposition_level, int extension,
                                                                    int[] decomposition_lengths, double[] output_data);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_denoising (double[] data, int data_len, int wavelet, int decomposition_level,
                                                            int wavelet_denoising, int threshold, int extenstion_type, int noise_level);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_window (int window_function, int window_len, double[] window_data);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_fft (double[] data, int data_len, int window, double[] re, double[] im);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ifft (double[] re, double[] im, int data_len, double[] data);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_nearest_power_of_two (int value, int[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_band_power (double[] ampls, double[] freqs, int data_len, double freq_start, double freq_end, double[] res);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detrend (double[] data, int len, int operation);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int remove_environmental_noise (double[] data, int len, int sampling_rate, int noise_type);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_custom_band_powers (double[] data, int rows, int cols, double[] start_freqs, double[] stop_freqs, int num_bands, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int calc_stddev (double[] data, int start_pos, int end_pos, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_railed_percentage (double[] data, int len, int gain, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_version_data_handler (byte[] version, int[] len, int max_len);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_oxygen_level (double[] ppg_ir, double[] ppg_red, int data_size, int sampling_rate, double coef1, double coef2, double coef3, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int restore_data_from_wavelet_detailed_coeffs (double[] data, int data_len, int wavelet, int decomposition_level, int level_to_restore, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detect_peaks_z_score (double[] data, int data_len, int lag, double threshold, double influence, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_heart_rate (double[] ppg_ir, double[] ppg_red, int data_size, int sampling_rate, int fft_size, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ica (double[] data, int rows, int cols, int num_components, double[] w, double[] k, double[] a, double[] s);
        // unsafe methods working with pointers
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_lowpass (double* data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_highpass (double* data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_bandpass (double* data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_bandstop (double* data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_rolling_filter (double* data, int len, int period, int operation);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_downsampling (double* data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_wavelet_transform (double* data, int data_len, int wavelet, int decomposition_level, int extension, double[] output_data, int[] decomposition_lengths);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_wavelet_denoising (double* data, int data_len, int wavelet, int decomposition_level,
                                                            int wavelet_denoising, int threshold, int extenstion_type, int noise_level);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_fft (double* data, int data_len, int window, double[] re, double[] im);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_psd (double* data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_psd_welch (double* data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int detrend (double* data, int len, int operation);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int remove_environmental_noise (double* data, int len, int sampling_rate, int noise_type);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_custom_band_powers (double* data, int rows, int cols, double[] start_freqs, double[] stop_freqs, int num_bands, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int calc_stddev (double* data, int start_pos, int end_pos, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_railed_percentage (double* data, int len, int gain, double[] output);
        [DllImport ("DataHandler", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int detect_peaks_z_score (double* data, int data_len, int lag, double threshold, double influence, double[] output);
    }

    class DataHandlerLibrary32
    {
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_data_handler (string log_file);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int log_message_data_handler (int log_level, string message);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_data_handler (int log_level);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_rolling_filter (double[] data, int len, int period, int operation);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_downsampling (double[] data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_elements_in_file (string file_name, int[] num_elements);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_transform (double[] data, int data_len, int wavelet, int decomposition_level, int extension, double[] output_data, int[] decomposition_lengths);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, int wavelet, int decomposition_level, int extension,
                                                                    int[] decomposition_lengths, double[] output_data);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_denoising (double[] data, int data_len, int wavelet, int decomposition_level,
                                                            int wavelet_denoising, int threshold, int extenstion_type, int noise_level);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_window (int window_function, int window_len, double[] window_data);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_fft (double[] data, int data_len, int window, double[] re, double[] im);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ifft (double[] re, double[] im, int data_len, double[] data);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_nearest_power_of_two (int value, int[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_band_power (double[] ampls, double[] freqs, int data_len, double freq_start, double freq_end, double[] res);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detrend (double[] data, int len, int operation);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_custom_band_powers (double[] data, int rows, int cols, double[] start_freqs, double[] stop_freqs, int num_bands, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int remove_environmental_noise (double[] data, int len, int sampling_rate, int noise_type);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int calc_stddev (double[] data, int start_pos, int end_pos, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_version_data_handler (byte[] version, int[] len, int max_len);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_railed_percentage (double[] data, int len, int gain, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_oxygen_level (double[] ppg_ir, double[] ppg_red, int data_size, int sampling_rate, double coef1, double coef2, double coef3, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int restore_data_from_wavelet_detailed_coeffs (double[] data, int data_len, int wavelet, int decomposition_level, int level_to_restore, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detect_peaks_z_score (double[] data, int data_len, int lag, double threshold, double influence, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_heart_rate (double[] ppg_ir, double[] ppg_red, int data_size, int sampling_rate, int fft_size, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ica (double[] data, int rows, int cols, int num_components, double[] w, double[] k, double[] a, double[] s);
        // unsafe methods working with pointers
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_lowpass (double* data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_highpass (double* data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_bandpass (double* data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_bandstop (double* data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_rolling_filter (double* data, int len, int period, int operation);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_downsampling (double* data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_wavelet_transform (double* data, int data_len, int wavelet, int decomposition_level, int extension, double[] output_data, int[] decomposition_lengths);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_wavelet_denoising (double* data, int data_len, int wavelet, int decomposition_level,
                                                            int wavelet_denoising, int threshold, int extenstion_type, int noise_level);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int perform_fft (double* data, int data_len, int window, double[] re, double[] im);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_psd (double* data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_psd_welch (double* data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int detrend (double* data, int len, int operation);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int remove_environmental_noise (double* data, int len, int sampling_rate, int noise_type);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_custom_band_powers (double* data, int rows, int cols, double[] start_freqs, double[] stop_freqs, int num_bands, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int calc_stddev (double* data, int start_pos, int end_pos, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int get_railed_percentage (double* data, int len, int gain, double[] output);
        [DllImport ("DataHandler32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int detect_peaks_z_score (double* data, int data_len, int lag, double threshold, double influence, double[] output);
    }

    class DataHandlerLibrary
    {
        public static int set_log_level_data_handler (int log_level)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.set_log_level_data_handler (log_level);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.set_log_level_data_handler (log_level);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int set_log_file_data_handler (string log_file)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.set_log_file_data_handler (log_file);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.set_log_file_data_handler (log_file);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int remove_environmental_noise (double[] data, int len, int sampling_rate, int noise_type)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.remove_environmental_noise (data, len, sampling_rate, noise_type);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.remove_environmental_noise (data, len, sampling_rate, noise_type);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_ica (double[] data, int rows, int cols, int num_components, double[] w, double[] k, double[] a, double[] s)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_ica (data, rows, cols, num_components, w, k, a, s);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_ica (data, rows, cols, num_components, w, k, a, s);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);

            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_bandpass (double[] data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_bandpass (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_bandpass (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_bandstop (double[] data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_bandstop (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_bandstop (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_rolling_filter (double[] data, int len, int period, int operation)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_rolling_filter (data, len, period, operation);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_rolling_filter (data, len, period, operation);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int restore_data_from_wavelet_detailed_coeffs (double[] data, int len, int wavelet, int decomposition_level, int level_to_restore, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.restore_data_from_wavelet_detailed_coeffs (data, len, wavelet, decomposition_level, level_to_restore, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.restore_data_from_wavelet_detailed_coeffs (data, len, wavelet, decomposition_level, level_to_restore, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int detect_peaks_z_score (double[] data, int len, int lag, double threshold, double influence, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.detect_peaks_z_score (data, len, lag, threshold, influence, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.detect_peaks_z_score (data, len, lag, threshold, influence, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int detrend (double[] data, int len, int operation)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.detrend (data, len, operation);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.detrend (data, len, operation);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_downsampling (double[] data, int len, int period, int operation, double[] filtered_data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_downsampling (data, len, period, operation, filtered_data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_downsampling (data, len, period, operation, filtered_data);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.read_file (data, num_rows, num_cols, file_name, max_elements);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.read_file (data, num_rows, num_cols, file_name, max_elements);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.write_file (data, num_rows, num_cols, file_name, file_mode);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.write_file (data, num_rows, num_cols, file_name, file_mode);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_num_elements_in_file (string file_name, int[] num_elements)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_num_elements_in_file (file_name, num_elements);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_num_elements_in_file (file_name, num_elements);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_nearest_power_of_two (int value, int[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_nearest_power_of_two (value, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_nearest_power_of_two (value, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_wavelet_transform (double[] data, int data_len, int wavelet, int decomposition_level, int extension, double[] output_data, int[] decomposition_lengths)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, extension, output_data, decomposition_lengths);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, extension, output_data, decomposition_lengths);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, int wavelet, int decomposition_level, int extension,
                                                                    int[] decomposition_lengths, double[] output_data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_inverse_wavelet_transform (wavelet_coeffs, original_data_len, wavelet, decomposition_level, extension, decomposition_lengths, output_data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_inverse_wavelet_transform (wavelet_coeffs, original_data_len, wavelet, decomposition_level, extension, decomposition_lengths, output_data);
             }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_wavelet_denoising (double[] data, int data_len, int wavelet, int decomposition_level,
                                                    int wavelet_denoising, int threshold, int extenstion_type, int noise_level)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level, wavelet_denoising, threshold, extenstion_type, noise_level);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level, wavelet_denoising, threshold, extenstion_type, noise_level);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_csp (data, labels, n_epochs, n_channels, n_times, output_filters, output_eigenvalues);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_csp (data, labels, n_epochs, n_channels, n_times, output_filters, output_eigenvalues);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_window (int window_function, int window_len, double[] window_data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_window (window_function, window_len, window_data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_window (window_function, window_len, window_data);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_fft (double[] data, int data_len, int window, double[] output_re, double[] output_im)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_fft (data, data_len, window, output_re, output_im);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_fft (data, data_len, window, output_re, output_im);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_custom_band_powers (double[] data, int rows, int cols, double[] start_freqs, double[] stop_freqs, int num_bands, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_custom_band_powers (data, rows, cols, start_freqs, stop_freqs, num_bands, sampling_rate, apply_filters, avgs, stddevs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_custom_band_powers (data, rows, cols, start_freqs, stop_freqs, num_bands, sampling_rate, apply_filters, avgs, stddevs);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] output_ampls, double[] output_freqs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_band_power (double[] ampls, double[] freqs, int data_len, double start_freq, double stop_freq, double[] res)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_band_power (ampls, freqs, data_len, start_freq, stop_freq, res);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_band_power (ampls, freqs, data_len, start_freq, stop_freq, res);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int perform_ifft (double[] re, double[] im, int len, double[] data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_ifft (re, im, len, data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_ifft (re, im, len, data);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] output_ampls, double[] output_freqs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int calc_stddev (double[] data, int start_pos, int end_pos, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.calc_stddev (data, start_pos, end_pos, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.calc_stddev (data, start_pos, end_pos, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_railed_percentage (double[] data, int len, int gain, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_railed_percentage (data, len, gain, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_railed_percentage (data, len, gain, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_oxygen_level (double[] ppg_ir, double[] ppg_red, int data_size, int sampling_rate, double coef1, double coef2, double coef3, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_oxygen_level (ppg_ir, ppg_red, data_size, sampling_rate, coef1, coef2, coef3, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_oxygen_level (ppg_ir, ppg_red, data_size, sampling_rate, coef1, coef2, coef3, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_heart_rate (double[] ppg_ir, double[] ppg_red, int data_size, int sampling_rate, int fft_size, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_heart_rate (ppg_ir, ppg_red, data_size, sampling_rate, fft_size, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_heart_rate (ppg_ir, ppg_red, data_size, sampling_rate, fft_size, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_version_data_handler (byte[] version, int[] len, int max_len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_version_data_handler (version, len, max_len);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_version_data_handler (version, len, max_len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int log_message_data_handler (int log_level, string message)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.log_message_data_handler (log_level, message);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.log_message_data_handler (log_level, message);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int remove_environmental_noise (double* data, int len, int sampling_rate, int noise_type)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.remove_environmental_noise (data, len, sampling_rate, noise_type);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.remove_environmental_noise (data, len, sampling_rate, noise_type);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_lowpass (double* data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_highpass (double* data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);

            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_bandpass (double* data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_bandpass (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_bandpass (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_bandstop (double* data, int len, int sampling_rate, double start_freq, double stop_freq, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_bandstop (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_bandstop (data, len, sampling_rate, start_freq, stop_freq, order, filter_type, ripple);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_rolling_filter (double* data, int len, int period, int operation)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_rolling_filter (data, len, period, operation);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_rolling_filter (data, len, period, operation);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int detect_peaks_z_score (double* data, int len, int lag, double threshold, double influence, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.detect_peaks_z_score (data, len, lag, threshold, influence, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.detect_peaks_z_score (data, len, lag, threshold, influence, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int detrend (double* data, int len, int operation)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.detrend (data, len, operation);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.detrend (data, len, operation);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_downsampling (double* data, int len, int period, int operation, double[] filtered_data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_downsampling (data, len, period, operation, filtered_data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_downsampling (data, len, period, operation, filtered_data);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_wavelet_transform (double* data, int data_len, int wavelet, int decomposition_level, int extension, double[] output_data, int[] decomposition_lengths)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, extension, output_data, decomposition_lengths);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, extension, output_data, decomposition_lengths);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_wavelet_denoising (double* data, int data_len, int wavelet, int decomposition_level,
                                                    int wavelet_denoising, int threshold, int extenstion_type, int noise_level)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level, wavelet_denoising, threshold, extenstion_type, noise_level);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level, wavelet_denoising, threshold, extenstion_type, noise_level);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int perform_fft (double* data, int data_len, int window, double[] output_re, double[] output_im)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_fft (data, data_len, window, output_re, output_im);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_fft (data, data_len, window, output_re, output_im);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int get_psd (double* data, int data_len, int sampling_rate, int window, double[] output_ampls, double[] output_freqs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int get_psd_welch (double* data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] output_ampls, double[] output_freqs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int calc_stddev (double* data, int start_pos, int end_pos, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.calc_stddev (data, start_pos, end_pos, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.calc_stddev (data, start_pos, end_pos, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static unsafe int get_railed_percentage (double* data, int len, int gain, double[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_railed_percentage (data, len, gain, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_railed_percentage (data, len, gain, output);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }
    }
}
