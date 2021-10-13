package brainflow;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.util.Arrays;

import org.apache.commons.lang3.SystemUtils;
import org.apache.commons.lang3.tuple.MutablePair;
import org.apache.commons.lang3.tuple.Pair;
import org.apache.commons.math3.complex.Complex;
import org.apache.commons.math3.transform.TransformUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;

/**
 * DataFilter class to perform signal processing
 */
public class DataFilter
{

    private interface DllInterface extends Library
    {
        int perform_lowpass (double[] data, int data_len, int sampling_rate, double cutoff, int order, int filter_type,
                double ripple);

        int perform_highpass (double[] data, int data_len, int sampling_rate, double cutoff, int order, int filter_type,
                double ripple);

        int perform_bandpass (double[] data, int data_len, int sampling_rate, double center_freq, double band_width,
                int order, int filter_type, double ripple);

        int perform_bandstop (double[] data, int data_len, int sampling_rate, double center_freq, double band_width,
                int order, int filter_type, double ripple);

        int perform_rolling_filter (double[] data, int data_len, int period, int operation);

        int detrend (double[] data, int data_len, int operation);

        int perform_downsampling (double[] data, int data_len, int period, int operation, double[] filtered_data);

        int remove_environmental_noise (double[] data, int data_len, int sampling_rate, int noise_type);

        int perform_wavelet_transform (double[] data, int data_len, String wavelet, int decomposition_level,
                double[] output_data, int[] decomposition_lengths);

        int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, String wavelet,
                int decomposition_level, int[] decomposition_lengths, double[] output_data);

        int perform_wavelet_denoising (double[] data, int data_len, String wavelet, int decomposition_level);

        int get_csp (double[] data, double[] labels, int n_epochs, int n_channels, int n_times, double[] output_filters,
                double[] output_eigenvalues);

        int get_window (int window_function, int window_len, double[] window_data);

        int perform_fft (double[] data, int data_len, int window, double[] output_re, double[] output_im);

        int perform_ifft (double[] re, double[] im, int data_len, double[] data);

        int write_file (double[] data, int num_rows, int num_cols, String file_name, String file_mode);

        int read_file (double[] data, int[] num_rows, int[] num_cols, String file_name, int max_elements);

        int set_log_level_data_handler (int log_level);

        int set_log_file_data_handler (String log_file);

        int get_num_elements_in_file (String file_name, int[] num_elements);

        int get_nearest_power_of_two (int value, int[] output);

        int get_psd (double[] data, int len, int sampling_rate, int window, double[] ampls, double[] freqs);

        int get_psd_welch (double[] data, int len, int nfft, int overlap, int sampling_rate, int window, double[] ampls,
                double[] freqs);

        int get_avg_band_powers (double[] data, int rows, int cols, int sampling_rate, int apply_filters, double[] avgs,
                double[] stddevs);

        int get_band_power (double[] ampls, double[] freqs, int len, double start_freq, double stop_freq,
                double[] output);
    }

    private static DllInterface instance;

    static
    {
        boolean is_os_android = "The Android Project".equals (System.getProperty ("java.specification.vendor"));

        String lib_name = "libDataHandler.so";
        if (SystemUtils.IS_OS_WINDOWS)
        {
            lib_name = "DataHandler.dll";

        } else if (SystemUtils.IS_OS_MAC)
        {
            lib_name = "libDataHandler.dylib";
        }

        if (is_os_android)
        {
            lib_name = "DataHandler";
        } else
        {
            // need to extract libraries from jar
            unpack_from_jar (lib_name);
        }
        instance = Native.loadLibrary (lib_name, DllInterface.class);
    }

    private static void unpack_from_jar (String lib_name)
    {
        try
        {
            File file = new File (lib_name);
            if (file.exists ())
                file.delete ();
            InputStream link = (BoardShim.class.getResourceAsStream (lib_name));
            Files.copy (link, file.getAbsoluteFile ().toPath ());
        } catch (Exception io)
        {
            System.err.println ("native library: " + lib_name + " is not found in jar file");
        }
    }

    /**
     * enable Data logger with level INFO
     */
    public static void enable_data_logger () throws BrainFlowError
    {
        set_log_level (2);
    }

    /**
     * enable Data logger with level TRACE
     */
    public static void enable_dev_data_logger () throws BrainFlowError
    {
        set_log_level (0);
    }

    /**
     * disable Data logger
     */
    public static void disable_data_logger () throws BrainFlowError
    {
        set_log_level (6);
    }

    /**
     * redirect logger from stderr to a file
     */
    public static void set_log_file (String log_file) throws BrainFlowError
    {
        int ec = instance.set_log_file_data_handler (log_file);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_file", ec);
        }
    }

    /**
     * set log level
     */
    private static void set_log_level (int log_level) throws BrainFlowError
    {
        int ec = instance.set_log_level_data_handler (log_level);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_level", ec);
        }
    }

    /**
     * perform lowpass filter in-place
     */
    public static void perform_lowpass (double[] data, int sampling_rate, double cutoff, int order, int filter_type,
            double ripple) throws BrainFlowError
    {
        int ec = instance.perform_lowpass (data, data.length, sampling_rate, cutoff, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    /**
     * perform highpass filter in-place
     */
    public static void perform_highpass (double[] data, int sampling_rate, double cutoff, int order, int filter_type,
            double ripple) throws BrainFlowError
    {
        int ec = instance.perform_highpass (data, data.length, sampling_rate, cutoff, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    /**
     * perform bandpass filter in-place
     */
    public static void perform_bandpass (double[] data, int sampling_rate, double center_freq, double band_width,
            int order, int filter_type, double ripple) throws BrainFlowError
    {
        int ec = instance.perform_bandpass (data, data.length, sampling_rate, center_freq, band_width, order,
                filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    /**
     * perform bandstop filter in-place
     */
    public static void perform_bandstop (double[] data, int sampling_rate, double center_freq, double band_width,
            int order, int filter_type, double ripple) throws BrainFlowError
    {
        int ec = instance.perform_bandstop (data, data.length, sampling_rate, center_freq, band_width, order,
                filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    /**
     * perform moving average or moving median filter in-place
     */
    public static void perform_rolling_filter (double[] data, int period, int operation) throws BrainFlowError
    {
        int ec = instance.perform_rolling_filter (data, data.length, period, operation);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    /**
     * subtract trend from data in-place
     */
    public static void detrend (double[] data, int operation) throws BrainFlowError
    {
        int ec = instance.detrend (data, data.length, operation);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to detrend", ec);
        }
    }

    /**
     * perform data downsampling, it doesnt apply lowpass filter for you, it just
     * aggregates several data points
     */
    public static double[] perform_downsampling (double[] data, int period, int operation) throws BrainFlowError
    {
        if (period <= 0)
        {
            throw new BrainFlowError ("Invalid period", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        if (data.length / period <= 0)
        {
            throw new BrainFlowError ("Invalid data size", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        double[] downsampled_data = new double[data.length / period];
        int ec = instance.perform_downsampling (data, data.length, period, operation, downsampled_data);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform downsampling", ec);
        }
        return downsampled_data;
    }

    /**
     * removes noise using notch filter
     */
    public static void remove_environmental_noise (double[] data, int sampling_rate, int noise_type)
            throws BrainFlowError
    {
        int ec = instance.remove_environmental_noise (data, data.length, sampling_rate, noise_type);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to remove noise", ec);
        }
    }

    /**
     * perform wavelet based denoising in-place
     * 
     * @param wavelet             supported vals:
     *                            db1..db15,haar,sym2..sym10,coif1..coif5,bior1.1,bior1.3,bior1.5,bior2.2,bior2.4,bior2.6,bior2.8,bior3.1,bior3.3,bior3.5
     *                            ,bior3.7,bior3.9,bior4.4,bior5.5,bior6.8
     * 
     * @param decomposition_level level of decomposition of wavelet transform
     */
    public static void perform_wavelet_denoising (double[] data, String wavelet, int decomposition_level)
            throws BrainFlowError
    {
        int ec = instance.perform_wavelet_denoising (data, data.length, wavelet, decomposition_level);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform denoising", ec);
        }
    }

    /**
     * perform wavelet transform
     * 
     * @param wavelet supported vals:
     *                db1..db15,haar,sym2..sym10,coif1..coif5,bior1.1,bior1.3,bior1.5,bior2.2,bior2.4,bior2.6,bior2.8,bior3.1,bior3.3,bior3.5
     *                ,bior3.7,bior3.9,bior4.4,bior5.5,bior6.8
     */
    public static Pair<double[], int[]> perform_wavelet_transform (double[] data, String wavelet,
            int decomposition_level) throws BrainFlowError
    {
        if (decomposition_level <= 0)
        {
            throw new BrainFlowError ("Invalid decomposition level", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        int[] lengths = new int[decomposition_level + 1];
        double[] output_array = new double[data.length + 2 * decomposition_level * (40 + 1)];
        int ec = instance.perform_wavelet_transform (data, data.length, wavelet, decomposition_level, output_array,
                lengths);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform wavelet transform", ec);
        }
        int total_sum = 0;
        for (int val : lengths)
        {
            total_sum += val;
        }
        Pair<double[], int[]> res = new MutablePair<double[], int[]> (Arrays.copyOfRange (output_array, 0, total_sum),
                lengths);
        return res;
    }

    /**
     * perform inverse wavelet transform
     */
    public static double[] perform_inverse_wavelet_transform (Pair<double[], int[]> wavelet_output,
            int original_data_len, String wavelet, int decomposition_level) throws BrainFlowError
    {
        if (decomposition_level <= 0)
        {
            throw new BrainFlowError ("Invalid decomposition level", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        double[] output_array = new double[original_data_len];
        int ec = instance.perform_inverse_wavelet_transform (wavelet_output.getLeft (), original_data_len, wavelet,
                decomposition_level, wavelet_output.getRight (), output_array);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform inverse wavelet transform", ec);
        }
        return output_array;
    }

    /**
     * get common spatial filters
     */
    public static Pair<double[][], double[]> get_csp (double[][][] data, double[] labels) throws BrainFlowError
    {
        int n_epochs = data.length;
        int n_channels = data[0].length;
        int n_times = data[0][0].length;

        double[] temp_data1d = new double[n_epochs * n_channels * n_times];
        for (int e = 0; e < n_epochs; e++)
        {
            for (int c = 0; c < n_channels; c++)
            {
                for (int t = 0; t < n_times; t++)
                {
                    int idx = e * n_channels * n_times + c * n_times + t;
                    temp_data1d[idx] = data[e][c][t];
                }
            }
        }

        double[] temp_filters = new double[n_channels * n_channels];
        double[] output_eigenvalues = new double[n_channels];

        int ec = instance.get_csp (temp_data1d, labels, n_epochs, n_channels, n_times, temp_filters,
                output_eigenvalues);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to get the CSP filters", ec);
        }

        double[][] output_filters = new double[n_channels][n_channels];
        for (int i = 0; i < n_channels; i++)
        {
            for (int j = 0; j < n_channels; j++)
            {
                output_filters[i][j] = temp_filters[i * n_channels + j];
            }
        }

        Pair<double[][], double[]> res = new MutablePair<double[][], double[]> (output_filters, output_eigenvalues);
        return res;
    }

    /**
     * perform data windowing
     * 
     * @param window     window function
     * @param window_len lenght of the window function
     * @return array of the size specified in window_len
     */
    public static double[] get_window (int window, int window_len) throws BrainFlowError
    {
        double[] window_data = new double[window_len];
        int ec = instance.get_window (window, window_len, window_data);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform windowing", ec);
        }
        return window_data;
    }

    /**
     * perform direct fft
     * 
     * @param data      data for fft transform
     * @param start_pos starting position to calc fft
     * @param end_pos   end position to calc fft, total_len must be a power of two
     * @param window    window function
     * @return array of complex values with size N / 2 + 1
     */
    public static Complex[] perform_fft (double[] data, int start_pos, int end_pos, int window) throws BrainFlowError
    {
        if ((start_pos < 0) || (end_pos > data.length) || (start_pos >= end_pos))
        {
            throw new BrainFlowError ("invalid position arguments", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        // I didnt find a way to pass an offset using pointers, copy array
        double[] data_to_process = Arrays.copyOfRange (data, start_pos, end_pos);
        int len = data_to_process.length;
        if ((len & (len - 1)) != 0)
        {
            throw new BrainFlowError ("end_pos - start_pos must be a power of 2",
                    ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        double[][] complex_array = new double[2][];
        complex_array[0] = new double[len / 2 + 1];
        complex_array[1] = new double[len / 2 + 1];
        int ec = instance.perform_fft (data_to_process, len, window, complex_array[0], complex_array[1]);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform fft", ec);
        }
        return TransformUtils.createComplexArray (complex_array);
    }

    /**
     * perform inverse fft
     * 
     * @param data data from fft transform(array of complex values)
     * @return restored data
     */
    public static double[] perform_ifft (Complex[] data) throws BrainFlowError
    {
        double[][] complex_array = TransformUtils.createRealImaginaryArray (data);
        int len = (data.length - 1) * 2;
        double[] output = new double[len];
        int ec = instance.perform_ifft (complex_array[0], complex_array[1], len, output);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform ifft", ec);
        }
        return output;
    }

    /**
     * calc average and stddev of band powers across all channels, bands are
     * 1-4,4-8,8-13,13-30,30-50
     * 
     * @param data          data to process
     * @param channels      rows of data arrays which should be used in calculation
     * @param sampling_rate sampling rate
     * @param apply_filters apply bandpass and bandstop filters before calculation
     * @return pair of avgs and stddevs for bandpowers
     */
    public static Pair<double[], double[]> get_avg_band_powers (double[][] data, int[] channels, int sampling_rate,
            boolean apply_filters) throws BrainFlowError
    {
        if ((data == null) || (channels == null))
        {
            throw new BrainFlowError ("data or channels null", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        double[] data_1d = new double[channels.length * data[channels[0]].length];
        for (int i = 0; i < channels.length; i++)
        {
            for (int j = 0; j < data[channels[i]].length; j++)
            {
                data_1d[j + i * data[channels[i]].length] = data[channels[i]][j];
            }
        }
        double[] avgs = new double[5];
        double[] stddevs = new double[5];
        int filters = (apply_filters) ? 1 : 0;
        int ec = instance.get_avg_band_powers (data_1d, channels.length, data[channels[0]].length, sampling_rate,
                filters, avgs, stddevs);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to get_avg_band_powers", ec);
        }
        Pair<double[], double[]> res = new MutablePair<double[], double[]> (avgs, stddevs);
        return res;
    }

    /**
     * get PSD
     * 
     * @param data          data to process
     * @param start_pos     starting position to calc PSD
     * @param end_pos       end position to calc PSD, total_len must be a power of
     *                      two
     * @param sampling_rate sampling rate
     * @param window        window function
     * @return pair of ampl and freq arrays with len N / 2 + 1
     */
    public static Pair<double[], double[]> get_psd (double[] data, int start_pos, int end_pos, int sampling_rate,
            int window) throws BrainFlowError
    {
        if ((start_pos < 0) || (end_pos > data.length) || (start_pos >= end_pos))
        {
            throw new BrainFlowError ("invalid position arguments", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        // I didnt find a way to pass an offset using pointers, copy array
        double[] data_to_process = Arrays.copyOfRange (data, start_pos, end_pos);
        int len = data_to_process.length;
        if ((len & (len - 1)) != 0)
        {
            throw new BrainFlowError ("end_pos - start_pos must be a power of 2",
                    ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        double[] ampls = new double[len / 2 + 1];
        double[] freqs = new double[len / 2 + 1];
        int ec = instance.get_psd (data_to_process, len, sampling_rate, window, ampls, freqs);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to get psd", ec);
        }
        Pair<double[], double[]> res = new MutablePair<double[], double[]> (ampls, freqs);
        return res;
    }

    /**
     * get PSD using Welch Method
     * 
     * @param data          data to process
     * @param nfft          size of FFT, must be power of two
     * @param overlap       overlap between FFT Windows, must be between 0 and nfft
     * @param sampling_rate sampling rate
     * @param window        window function
     * @return pair of ampl and freq arrays
     */
    public static Pair<double[], double[]> get_psd_welch (double[] data, int nfft, int overlap, int sampling_rate,
            int window) throws BrainFlowError
    {
        if ((nfft & (nfft - 1)) != 0)
        {
            throw new BrainFlowError ("nfft must be a power of 2", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        double[] ampls = new double[nfft / 2 + 1];
        double[] freqs = new double[nfft / 2 + 1];
        int ec = instance.get_psd_welch (data, data.length, nfft, overlap, sampling_rate, window, ampls, freqs);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to get_psd_welch", ec);
        }
        Pair<double[], double[]> res = new MutablePair<double[], double[]> (ampls, freqs);
        return res;
    }

    /**
     * get band power
     * 
     * @param psd        PSD from get_psd or get_log_psd
     * @param freq_start lowest frequency of band
     * @param freq_end   highest frequency of band
     * @return band power
     */
    public static double get_band_power (Pair<double[], double[]> psd, double freq_start, double freq_end)
            throws BrainFlowError
    {
        double[] res = new double[1];
        int ec = instance.get_band_power (psd.getLeft (), psd.getRight (), psd.getLeft ().length, freq_start, freq_end,
                res);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to get band power", ec);
        }
        return res[0];
    }

    /**
     * calculate nearest power of two
     */
    public static int get_nearest_power_of_two (int value) throws BrainFlowError
    {
        int[] power_of_two = new int[1];
        int ec = instance.get_nearest_power_of_two (value, power_of_two);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to calc nearest power of two", ec);
        }
        return power_of_two[0];
    }

    /**
     * write data to tsv file, in file data will be transposed
     */
    public static void write_file (double[][] data, String file_name, String file_mode) throws BrainFlowError
    {
        if (data.length == 0)
        {
            throw new BrainFlowError ("empty data array", ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        double[] linear_data = reshape_data_to_1d (data.length, data[0].length, data);
        int ec = instance.write_file (linear_data, data.length, data[0].length, file_name, file_mode);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to write data to file", ec);
        }
    }

    /**
     * read data from file, transpose it back to original format
     */
    public static double[][] read_file (String file_name) throws BrainFlowError
    {
        int[] num_elements = new int[1];
        int ec = instance.get_num_elements_in_file (file_name, num_elements);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to determine number of elements in a file", ec);
        }
        double[] data_arr = new double[num_elements[0]];
        int[] num_rows = new int[1];
        int[] num_cols = new int[1];
        ec = instance.read_file (data_arr, num_rows, num_cols, file_name, num_elements[0]);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to read data to file", ec);
        }
        return reshape_data_to_2d (num_rows[0], num_cols[0], data_arr);
    }

    private static double[] reshape_data_to_1d (int num_rows, int num_cols, double[][] buf)
    {
        double[] output_buf = new double[num_rows * num_cols];
        for (int i = 0; i < num_cols; i++)
        {
            for (int j = 0; j < num_rows; j++)
            {
                output_buf[j * num_cols + i] = buf[j][i];
            }
        }
        return output_buf;
    }

    private static double[][] reshape_data_to_2d (int num_rows, int num_cols, double[] linear_buffer)
    {
        double[][] result = new double[num_rows][];
        for (int i = 0; i < num_rows; i++)
        {
            result[i] = Arrays.copyOfRange (linear_buffer, (i * num_cols), (i + 1) * num_cols);
        }
        return result;
    }

}
