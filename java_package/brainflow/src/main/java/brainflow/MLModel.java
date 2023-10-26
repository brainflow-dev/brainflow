package brainflow;

import java.nio.file.Path;
import java.util.Arrays;

import org.apache.commons.lang3.SystemUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;

@SuppressWarnings ("deprecation")
public class MLModel
{
    private interface DllInterface extends Library
    {

        int set_log_level_ml_module (int log_level);

        int set_log_file_ml_module (String log_file);

        int log_message_ml_module (int log_level, String message);

        int prepare (String params);

        int release (String params);

        int predict (double[] data, int data_len, double[] output, int[] output_len, String params);

        int release_all ();

        int get_version_ml_module (byte[] version, int[] len, int max_len);
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
            String arch = System.getProperty ("os.arch");
            switch (arch) {
                case "x86":
                    JarHelper.unpack_from_jar ("onnxruntime_x86.dll");
                    break;
                case "x86_64":
                case "amd64":
                    JarHelper.unpack_from_jar ("onnxruntime_x64.dll");
                    break;
                case "arm":
                    JarHelper.unpack_from_jar ("onnxruntime_arm.dll");
                    break;
                case "arm64":
                    JarHelper.unpack_from_jar ("onnxruntime_arm64.dll");
                    break;
                default:
                    System.err.println("Unsupported Windows architecture: " + arch);
            }
        } else if (SystemUtils.IS_OS_MAC)
        {
            lib_name = "libMLModule.dylib";
            JarHelper.unpack_from_jar ("onnxruntime_x86.dll");
            JarHelper.unpack_from_jar ("onnxruntime_x86.dll");
        } else if ((SystemUtils.IS_OS_LINUX) && (!is_os_android))
        {
            JarHelper.unpack_from_jar ("libonnxruntime_x64.so");
            JarHelper.unpack_from_jar ("libonnxruntime_arm64.so");
        }

        if (is_os_android)
        {
            // for android you need to put these files manually to jniLibs folder, unpacking
            // doesnt work
            lib_name = "MLModule"; // no lib prefix and no extension for android
        } else
        {
            // need to extract libraries from jar
            Path lib_path = JarHelper.unpack_from_jar (lib_name);
            if (lib_path != null)
            {
                lib_name = lib_path.toString ();
            }
        }

        instance = Native.loadLibrary (lib_name, DllInterface.class);
    }

    private String input_params;

    private BrainFlowModelParams params;

    /**
     * Create MLModel object
     */
    public MLModel (BrainFlowModelParams params)
    {
        input_params = params.to_json ();
        this.params = params;
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
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_file", ec);
        }
    }

    /**
     * Get version
     */
    public static String get_version () throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[64];
        int ec = instance.get_version_ml_module (str, len, 64);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in get_version", ec);
        }
        String version = new String (str, 0, len[0]);
        return version;
    }

    /**
     * release all classifiers
     */
    public static void release_all () throws BrainFlowError
    {
        int ec = instance.release_all ();
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in release classifiers", ec);
        }
    }

    /**
     * send user defined strings to BrainFlow logger
     */
    public static void log_message (int log_level, String message) throws BrainFlowError
    {
        int ec = instance.log_message_ml_module (log_level, message);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in log_message", ec);
        }
    }

    /**
     * set log level
     */
    public static void set_log_level (int log_level) throws BrainFlowError
    {
        int ec = instance.set_log_level_ml_module (log_level);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_level", ec);
        }
    }

    /**
     * set log level
     */
    public static void set_log_level (LogLevels log_level) throws BrainFlowError
    {
        set_log_level (log_level.get_code ());
    }

    /**
     * Prepare classifier
     * 
     * @throws BrainFlowError
     */
    public void prepare () throws BrainFlowError
    {
        int ec = instance.prepare (input_params);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
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
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in release", ec);
        }
    }

    /**
     * Get score of classifier
     * 
     * @throws BrainFlowError
     */
    public double[] predict (double[] data) throws BrainFlowError
    {
        double[] val = new double[params.max_array_size];
        int[] val_len = new int[1];
        int ec = instance.predict (data, data.length, val, val_len, input_params);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in predict", ec);
        }
        return Arrays.copyOfRange (val, 0, val_len[0]);
    }
}
