package brainflow;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;

import org.apache.commons.lang3.SystemUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;

public class MLModel
{
    private interface DllInterface extends Library
    {

        int set_log_level_ml_module (int log_level);

        int set_log_file_ml_module (String log_file);

        int prepare (String params);

        int release (String params);

        int predict (double[] data, int data_len, double[] output, String params);
    }

    private static DllInterface instance;

    static
    {
        // SystemUtils doesnt have smth like IS_OS_ANDROID, need to check using
        // properties
        boolean is_os_android = "The Android Project".equals (System.getProperty ("java.specification.vendor"));

        String lib_name = "libMLModule.so";
        if (SystemUtils.IS_OS_WINDOWS)
        {
            lib_name = "MLModule.dll";

        } else if (SystemUtils.IS_OS_MAC)
        {
            lib_name = "libMLModule.dylib";
        }

        if (is_os_android)
        {
            // for android you need to put these files manually to jniLibs folder, unpacking
            // doesnt work
            lib_name = "MLModule"; // no lib prefix and no extension for android
        } else
        {
            // need to extract libraries from jar
            unpack_from_jar (lib_name);
            unpack_from_jar ("brainflow_svm.model");
        }

        instance = Native.loadLibrary (lib_name, DllInterface.class);
    }

    private static Path unpack_from_jar (String lib_name)
    {
        try
        {
            File file = new File (lib_name);
            if (file.exists ())
                file.delete ();
            InputStream link = (BoardShim.class.getResourceAsStream (lib_name));
            Files.copy (link, file.getAbsoluteFile ().toPath ());
            return file.getAbsoluteFile ().toPath ();
        } catch (Exception io)
        {
            System.err.println ("file: " + lib_name + " is not found in jar file");
            return null;
        }
    }

    private String input_params;

    /**
     * Create MLModel object
     */
    public MLModel (BrainFlowModelParams params)
    {
        input_params = params.to_json ();
    }

    /**
     * enable ML logger with level INFO
     */
    public static void enable_ml_logger () throws BrainFlowError
    {
        set_log_level (2);
    }

    /**
     * enable ML logger with level TRACE
     */
    public static void enable_dev_ml_logger () throws BrainFlowError
    {
        set_log_level (0);
    }

    /**
     * disable BrainFlow logger
     */
    public static void disable_ml_logger () throws BrainFlowError
    {
        set_log_level (6);
    }

    /**
     * redirect logger from stderr to a file
     */
    public static void set_log_file (String log_file) throws BrainFlowError
    {
        int ec = instance.set_log_file_ml_module (log_file);
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
        int ec = instance.set_log_level_ml_module (log_level);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_level", ec);
        }
    }

    /**
     * Prepare classifier
     * 
     * @throws BrainFlowError
     */
    public void prepare () throws BrainFlowError
    {
        int ec = instance.prepare (input_params);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in prepare", ec);
        }
    }

    /**
     * Release classifier
     * 
     * @throws BrainFlowError
     */
    public void release () throws BrainFlowError
    {
        int ec = instance.release (input_params);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in release", ec);
        }
    }

    /**
     * Get score of classifier
     * 
     * @throws BrainFlowError
     */
    public double predict (double[] data) throws BrainFlowError
    {
        double[] val = new double[1];
        int ec = instance.predict (data, data.length, val, input_params);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in predict", ec);
        }
        return val[0];
    }
}
