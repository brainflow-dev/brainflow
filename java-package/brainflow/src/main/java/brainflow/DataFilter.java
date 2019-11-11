package brainflow;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.util.Arrays;

import org.apache.commons.lang3.SystemUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;

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

    public static void perform_lowpass (double[] data, int sampling_rate, double cutoff, int order, int filter_type,
            double ripple) throws BrainFlowError
    {
        int ec = instance.perform_lowpass (data, data.length, sampling_rate, cutoff, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    public static void perform_highpass (double[] data, int sampling_rate, double cutoff, int order, int filter_type,
            double ripple) throws BrainFlowError
    {
        int ec = instance.perform_highpass (data, data.length, sampling_rate, cutoff, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

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
