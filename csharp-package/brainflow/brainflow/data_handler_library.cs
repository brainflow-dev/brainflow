using System.Runtime.InteropServices;

namespace brainflow
{
    public enum FilterTypes
    {
        BUTTERWORTH = 0,
        CHEBYSHEV_TYPE_1 = 1,
        BESSEL = 2
    };

    public enum AggOperations
    {
        MEAN = 0,
        MEDIAN = 1,
        EACH = 2
    };

    public enum WindowFunctions
    {
        NO_WINDOW = 0,
        HANNING = 1,
        HAMMING = 2,
        BLACKMAN_HARRIS = 3
    };

    public enum DetrendOperations
    {
        NONE = 0,
        CONSTANT = 1,
        LINEAR = 2
    };

    public enum NoiseTypes
    {
        FIFTY = 0,
        SIXTY = 1
    };

    class DataHandlerLibrary64
    {
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_data_handler (string log_file);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_data_handler (int log_level);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_rolling_filter (double[] data, int len, int period, int operation);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_downsampling (double[] data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_elements_in_file (string file_name, int[] num_elements);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_transform (double[] data, int data_len, string wavelet, int decomposition_level, double[] output_data, int[] decomposition_lengths);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, string wavelet, int decomposition_level,
                                                                    int[] decomposition_lengths, double[] output_data);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_denoising (double[] data, int data_len, string wavelet, int decomposition_level);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_window (int window_function, int window_len, double[] window_data);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_fft (double[] data, int data_len, int window, double[] re, double[] im);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ifft (double[] re, double[] im, int data_len, double[] data);
        [DllImport("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_nearest_power_of_two (int value, int[] output);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_band_power (double[] ampls, double[] freqs, int data_len, double freq_start, double freq_end, double[] res);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detrend (double[] data, int len, int operation);
        [DllImport("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int remove_environmental_noise(double[] data, int len, int sampling_rate, int noise_type);
        [DllImport ("DataHandler.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_avg_band_powers (double[] data, int rows, int cols, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
    }

    class DataHandlerLibrary32
    {
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_data_handler(string log_file);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_data_handler (int log_level);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_rolling_filter (double[] data, int len, int period, int operation);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_downsampling (double[] data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_elements_in_file (string file_name, int[] num_elements);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_transform (double[] data, int data_len, string wavelet, int decomposition_level, double[] output_data, int[] decomposition_lengths);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, string wavelet, int decomposition_level,
                                                                    int[] decomposition_lengths, double[] output_data);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_denoising (double[] data, int data_len, string wavelet, int decomposition_level);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_window (int window_function, int window_len, double[] window_data);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_fft (double[] data, int data_len, int window, double[] re, double[] im);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ifft (double[] re, double[] im, int data_len, double[] data);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_nearest_power_of_two (int value, int[] output);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_band_power (double[] ampls, double[] freqs, int data_len, double freq_start, double freq_end, double[] res);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detrend (double[] data, int len, int operation);
        [DllImport ("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_avg_band_powers (double[] data, int rows, int cols, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
        [DllImport("DataHandler32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int remove_environmental_noise(double[] data, int len, int sampling_rate, int noise_type);
    }

    class DataHandlerLibraryLinux
    {
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_data_handler(string log_file);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_data_handler (int log_level);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_rolling_filter (double[] data, int len, int period, int operation);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_downsampling (double[] data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_elements_in_file (string file_name, int[] num_elements);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_transform (double[] data, int data_len, string wavelet, int decomposition_level, double[] output_data, int[] decomposition_lengths);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, string wavelet, int decomposition_level,
                                                                    int[] decomposition_lengths, double[] output_data);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_denoising (double[] data, int data_len, string wavelet, int decomposition_level);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_window (int window_function, int window_len, double[] window_data);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_fft (double[] data, int data_len, int window, double[] re, double[] im);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ifft (double[] re, double[] im, int data_len, double[] data);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_nearest_power_of_two (int value, int[] output);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_band_power (double[] ampls, double[] freqs, int data_len, double freq_start, double freq_end, double[] res);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detrend (double[] data, int len, int operation);
        [DllImport ("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_avg_band_powers (double[] data, int rows, int cols, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
        [DllImport("libDataHandler.so", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int remove_environmental_noise(double[] data, int len, int sampling_rate, int noise_type);
    }

    class DataHandlerLibraryMac
    {
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_data_handler(string log_file);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_data_handler(int log_level);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_rolling_filter (double[] data, int len, int period, int operation);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_downsampling (double[] data, int len, int period, int operation, double[] downsampled_data);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_elements_in_file (string file_name, int[] num_elements);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_transform (double[] data, int data_len, string wavelet, int decomposition_level, double[] output_data, int[] decomposition_lengths);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, string wavelet, int decomposition_level,
                                                                    int[] decomposition_lengths, double[] output_data);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_wavelet_denoising (double[] data, int data_len, string wavelet, int decomposition_level);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_window (int window_function, int window_len, double[] window_data);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_fft (double[] data, int data_len, int window, double[] re, double[] im);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int perform_ifft (double[] re, double[] im, int data_len, double[] data);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_nearest_power_of_two (int value, int[] output);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_band_power (double[] ampls, double[] freqs, int data_len, double freq_start, double freq_end, double[] res);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] ampls, double[] freqs);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int detrend (double[] data, int len, int operation);
        [DllImport ("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_avg_band_powers (double[] data, int rows, int cols, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs);
        [DllImport("libDataHandler.dylib", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int remove_environmental_noise(double[] data, int len, int sampling_rate, int noise_type);
    }

    class DataHandlerLibrary
    {
        public static int set_log_level_data_handler (int log_level)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.set_log_level_data_handler(log_level);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.set_log_level_data_handler(log_level);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.set_log_level_data_handler(log_level);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.set_log_level_data_handler(log_level);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int set_log_file_data_handler (string log_file)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.set_log_file_data_handler(log_file);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.set_log_file_data_handler(log_file);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.set_log_file_data_handler(log_file);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.set_log_file_data_handler(log_file);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int remove_environmental_noise(double[] data, int len, int sampling_rate, int noise_type)
        {
            switch (PlatformHelper.get_library_environment())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.remove_environmental_noise(data, len, sampling_rate, noise_type);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.remove_environmental_noise(data, len, sampling_rate, noise_type);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.remove_environmental_noise(data, len, sampling_rate, noise_type);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.remove_environmental_noise(data, len, sampling_rate, noise_type);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_lowpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_lowpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_highpass (double[] data, int len, int sampling_rate, double cutoff, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_highpass (data, len, sampling_rate, cutoff, order, filter_type, ripple);

            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_bandpass (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_bandpass (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_bandpass (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_bandpass (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_bandpass (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_bandstop (double[] data, int len, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_bandstop (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_bandstop (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_bandstop (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_bandstop (data, len, sampling_rate, center_freq, band_width, order, filter_type, ripple);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_rolling_filter (double[] data, int len, int period, int operation)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_rolling_filter (data, len, period, operation);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_rolling_filter (data, len, period, operation);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_rolling_filter (data, len, period, operation);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_rolling_filter (data, len, period, operation);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int detrend (double[] data, int len, int operation)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.detrend (data, len, operation);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.detrend (data, len, operation);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.detrend (data, len, operation);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.detrend (data, len, operation);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_downsampling (double[] data, int len, int period, int operation, double[] filtered_data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_downsampling (data, len, period, operation, filtered_data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_downsampling (data, len, period, operation, filtered_data);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_downsampling (data, len, period, operation, filtered_data);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_downsampling (data, len, period, operation, filtered_data);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int read_file (double[] data, int[] num_rows, int[] num_cols, string file_name, int max_elements)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.read_file (data, num_rows, num_cols, file_name, max_elements);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.read_file (data, num_rows, num_cols, file_name, max_elements);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.read_file (data, num_rows, num_cols, file_name, max_elements);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.read_file (data, num_rows, num_cols, file_name, max_elements);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int write_file (double[] data, int num_rows, int num_cols, string file_name, string file_mode)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.write_file (data, num_rows, num_cols, file_name, file_mode);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.write_file (data, num_rows, num_cols, file_name, file_mode);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.write_file (data, num_rows, num_cols, file_name, file_mode);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.write_file (data, num_rows, num_cols, file_name, file_mode);

            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_num_elements_in_file (string file_name, int[] num_elements)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_num_elements_in_file (file_name, num_elements);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_num_elements_in_file (file_name, num_elements);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_num_elements_in_file (file_name, num_elements);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_num_elements_in_file (file_name, num_elements);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_nearest_power_of_two (int value, int[] output)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_nearest_power_of_two (value, output);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_nearest_power_of_two (value, output);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_nearest_power_of_two (value, output);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_nearest_power_of_two (value, output);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_wavelet_transform (double[] data, int data_len, string wavelet, int decomposition_level, double[] output_data, int[] decomposition_lengths)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, output_data, decomposition_lengths);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, output_data, decomposition_lengths);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, output_data, decomposition_lengths);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_wavelet_transform (data, data_len, wavelet, decomposition_level, output_data, decomposition_lengths);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, string wavelet, int decomposition_level,
                                                                    int[] decomposition_lengths, double[] output_data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_inverse_wavelet_transform (wavelet_coeffs, original_data_len, wavelet, decomposition_level, decomposition_lengths, output_data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_inverse_wavelet_transform (wavelet_coeffs, original_data_len, wavelet, decomposition_level, decomposition_lengths, output_data);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_inverse_wavelet_transform (wavelet_coeffs, original_data_len, wavelet, decomposition_level, decomposition_lengths, output_data);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_inverse_wavelet_transform (wavelet_coeffs, original_data_len, wavelet, decomposition_level, decomposition_lengths, output_data);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_wavelet_denoising (double[] data, int data_len, string wavelet, int decomposition_level)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_wavelet_denoising (data, data_len, wavelet, decomposition_level);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters, double[] output_eigenvalues)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_csp (data, labels, n_epochs, n_channels, n_times, output_filters, output_eigenvalues);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_csp (data, labels, n_epochs, n_channels, n_times, output_filters, output_eigenvalues);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_csp (data, labels, n_epochs, n_channels, n_times, output_filters, output_eigenvalues);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_csp (data, labels, n_epochs, n_channels, n_times, output_filters, output_eigenvalues);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_window (int window_function, int window_len, double[] window_data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_window (window_function, window_len, window_data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_window (window_function, window_len, window_data);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_window (window_function, window_len, window_data);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_window (window_function, window_len, window_data);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_fft (double[] data, int data_len, int window, double[] output_re, double[] output_im)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_fft (data, data_len, window, output_re, output_im);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_fft (data, data_len, window, output_re, output_im);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_fft (data, data_len, window, output_re, output_im);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_fft (data, data_len, window, output_re, output_im);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_avg_band_powers (double[] data, int rows, int cols, int sampling_rate, int apply_filters, double[] avgs, double[] stddevs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_avg_band_powers (data, rows, cols, sampling_rate, apply_filters, avgs, stddevs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_avg_band_powers (data, rows, cols, sampling_rate, apply_filters, avgs, stddevs);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_avg_band_powers (data, rows, cols, sampling_rate, apply_filters, avgs, stddevs);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_avg_band_powers (data, rows, cols, sampling_rate, apply_filters, avgs, stddevs);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_psd (double[] data, int data_len, int sampling_rate, int window, double[] output_ampls, double[] output_freqs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_psd (data, data_len, sampling_rate, window, output_ampls, output_freqs);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_band_power (double[] ampls, double[] freqs, int data_len, double start_freq, double stop_freq, double[] res)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_band_power (ampls, freqs, data_len, start_freq, stop_freq, res);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_band_power (ampls, freqs, data_len, start_freq, stop_freq, res);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_band_power (ampls, freqs, data_len, start_freq, stop_freq, res);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_band_power (ampls, freqs, data_len, start_freq, stop_freq, res);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int perform_ifft (double[] re, double[] im, int len, double[] data)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.perform_ifft (re, im, len, data);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.perform_ifft (re, im, len, data);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.perform_ifft (re, im, len, data);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.perform_ifft (re, im, len, data);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }

        public static int get_psd_welch (double[] data, int data_len, int nfft, int overlap, int sampling_rate, int window, double[] output_ampls, double[] output_freqs)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return DataHandlerLibrary64.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.x86:
                    return DataHandlerLibrary32.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.Linux:
                    return DataHandlerLibraryLinux.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
                case LibraryEnvironment.MacOS:
                    return DataHandlerLibraryMac.get_psd_welch (data, data_len, nfft, overlap, sampling_rate, window, output_ampls, output_freqs);
            }

            return (int)CustomExitCodes.GENERAL_ERROR;
        }
    }
}
