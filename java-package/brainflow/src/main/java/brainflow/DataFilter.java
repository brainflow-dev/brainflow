package brainflow;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.util.Arrays;

import org.apache.commons.lang3.SystemUtils;
import org.apache.commons.lang3.tuple.MutablePair;
import org.apache.commons.lang3.tuple.Pair;

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

        int perform_downsampling (double[] data, int data_len, int period, int operation, double[] filtered_data);

        int perform_wavelet_transform (double[] data, int data_len, String wavelet, int decomposition_level,
                double[] output_data, int[] decomposition_lengths);

        int perform_inverse_wavelet_transform (double[] wavelet_coeffs, int original_data_len, String wavelet,
                int decomposition_level, int[] decomposition_lengths, double[] output_data);

        int perform_wavelet_denoising (double[] data, int data_len, String wavelet, int decomposition_level);

        int write_file (double[] data, int num_rows, int num_cols, String file_name, String file_mode);

        int read_file (double[] data, int[] num_rows, int[] num_cols, String file_name, int max_elements);

        int get_num_elements_in_file (String file_name, int[] num_elements);
    }

    private static DllInterface instance;

    static
    {
        String lib_name = "libDataHandler.so";
        if (SystemUtils.IS_OS_WINDOWS)
        {
            lib_name = "DataHandler.dll";

        } else if (SystemUtils.IS_OS_MAC)
        {
            lib_name = "libDataHandler.dylib";
        }

        // need to extract libraries from jar
        unpack_from_jar (lib_name);
        instance = (DllInterface) Native.loadLibrary (lib_name, DllInterface.class);
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
        double[] downsampled_data = new double[(int) (data.length / period)];
        int ec = instance.perform_downsampling (data, data.length, period, operation, downsampled_data);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to perform downsampling", ec);
        }
        return downsampled_data;
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
     * write data to csv file, in file data will be transposed
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
