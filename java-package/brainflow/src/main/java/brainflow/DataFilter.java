package brainflow;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.io.IOException;
import java.util.Arrays;

import org.apache.commons.lang3.SystemUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;


public class DataFilter {

    private interface DllInterface extends Library {
        int perform_lowpass (double[] data, int data_len,
                int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        int perform_highpass (double[] data, int data_len,
                int sampling_rate, double cutoff, int order, int filter_type, double ripple);
        int perform_bandpass (double[] data, int data_len,
                int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
        int perform_bandstop (double[] data, int data_len,
                int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple);
    }
    private static DllInterface instance;

    static {
        String lib_name = "libDataHandler.so";
        if (SystemUtils.IS_OS_WINDOWS) {
            lib_name = "DataHandler.dll";
            
        }
        else if (SystemUtils.IS_OS_MAC) {
            lib_name = "libDataHandler.dylib";
        }

        // need to extract libraries from jar
        unpack_from_jar (lib_name);
        instance = (DllInterface) Native.loadLibrary (lib_name, DllInterface.class);
    }
    
    private static void unpack_from_jar (String lib_name)
    {
        try {
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
    
    public static void perform_lowpass (double[] data, int sampling_rate, double cutoff, int order, int filter_type, double ripple) throws BrainFlowError {
        int ec = instance.perform_lowpass (data, data.length, sampling_rate, cutoff, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    public static void perform_highpass (double[] data, int sampling_rate, double cutoff, int order, int filter_type, double ripple) throws BrainFlowError {
        int ec = instance.perform_highpass (data, data.length, sampling_rate, cutoff, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }

    public static void perform_bandpass (double[] data, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple) throws BrainFlowError {
        int ec = instance.perform_bandpass (data, data.length, sampling_rate, center_freq, band_width, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }
    
    public static void perform_bandstop (double[] data, int sampling_rate, double center_freq, double band_width, int order, int filter_type, double ripple) throws BrainFlowError {
        int ec = instance.perform_bandstop (data, data.length, sampling_rate, center_freq, band_width, order, filter_type, ripple);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Failed to apply filter", ec);
        }
    }
    
}
