package brainflow;

import java.io.IOException;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Collections;

import org.apache.commons.lang3.SystemUtils;

import com.google.gson.Gson;
import com.sun.jna.JNIEnv;
import com.sun.jna.Library;
import com.sun.jna.Native;

/**
 * BoardShim class to communicate with a board
 */
@SuppressWarnings ("deprecation")
public class BoardShim
{

    private interface DllInterface extends Library
    {
        int prepare_session (int board_id, String params);

        int config_board (String config, byte[] names, int[] len, int board_id, String params);

        int config_board_with_bytes (byte[] bytes, int len, int board_id, String params);

        int add_streamer (String streamer, int preset, int board_id, String params);

        int delete_streamer (String streamer, int preset, int board_id, String params);

        int start_stream (int buffer_size, String streamer_params, int board_id, String params);

        int stop_stream (int board_id, String params);

        int release_session (int board_id, String params);

        int insert_marker (double value, int preset, int board_id, String params);

        int get_current_board_data (int num_samples, int preset, double[] data_buf, int[] returned_samples,
                int board_id, String params);

        int get_board_data_count (int preset, int[] result, int board_id, String params);

        int get_board_data (int data_count, int preset, double[] data_buf, int board_id, String params);

        int set_log_level_board_controller (int log_level);

        int log_message_board_controller (int log_level, String message);

        int set_log_file_board_controller (String log_file);

        int java_set_jnienv (JNIEnv java_jnienv);

        int get_sampling_rate (int board_id, int preset, int[] sampling_rate);

        int get_battery_channel (int board_id, int preset, int[] battery_channel);

        int get_package_num_channel (int board_id, int preset, int[] package_num_channel);

        int get_num_rows (int board_id, int preset, int[] num_rows);

        int get_timestamp_channel (int board_id, int preset, int[] timestamp_channel);

        int get_marker_channel (int board_id, int preset, int[] marker_channel);

        int get_eeg_channels (int board_id, int preset, int[] eeg_channels, int[] len);

        int get_exg_channels (int board_id, int preset, int[] eeg_channels, int[] len);

        int get_emg_channels (int board_id, int preset, int[] emg_channels, int[] len);

        int get_ecg_channels (int board_id, int preset, int[] ecg_channels, int[] len);

        int get_eog_channels (int board_id, int preset, int[] eog_channels, int[] len);

        int get_eda_channels (int board_id, int preset, int[] eda_channels, int[] len);

        int get_ppg_channels (int board_id, int preset, int[] ppg_channels, int[] len);

        int get_accel_channels (int board_id, int preset, int[] accel_channels, int[] len);

        int get_rotation_channels (int board_id, int preset, int[] rotation_channels, int[] len);

        int get_analog_channels (int board_id, int preset, int[] analog_channels, int[] len);

        int get_gyro_channels (int board_id, int preset, int[] gyro_channels, int[] len);

        int get_other_channels (int board_id, int preset, int[] other_channels, int[] len);

        int get_resistance_channels (int board_id, int preset, int[] channels, int[] len);

        int get_magnetometer_channels (int board_id, int preset, int[] channels, int[] len);

        int get_temperature_channels (int board_id, int preset, int[] temperature_channels, int[] len);

        int release_all_sessions ();

        int is_prepared (int[] prepared, int board_id, String params);

        int get_eeg_names (int board_id, int preset, byte[] names, int[] len);

        int get_board_descr (int board_id, int preset, byte[] names, int[] len);

        int get_device_name (int board_id, int preset, byte[] name, int[] len);

        int get_board_presets (int board_id, int[] presets, int[] len);

        int get_version_board_controller (byte[] version, int[] len, int max_len);
    }

    private static DllInterface instance;

    static
    {
        // SystemUtils doesnt have smth like IS_OS_ANDROID, need to check using
        // properties
        boolean is_os_android = "The Android Project".equals (System.getProperty ("java.specification.vendor"));

        String lib_name = "libBoardController.so";
        if (SystemUtils.IS_OS_WINDOWS)
        {
            lib_name = "BoardController.dll";
            JarHelper.unpack_from_jar ("neurosdk-x64.dll");
            JarHelper.unpack_from_jar ("Unicorn.dll");
            JarHelper.unpack_from_jar ("gForceSDKWrapper.dll");
            JarHelper.unpack_from_jar ("gforce64.dll");
            JarHelper.unpack_from_jar ("simpleble-c.dll");
            JarHelper.unpack_from_jar ("MuseLib.dll");
            JarHelper.unpack_from_jar ("BrainBitLib.dll");
            JarHelper.unpack_from_jar ("GanglionLib.dll");
            JarHelper.unpack_from_jar ("BrainFlowBluetooth.dll");
            JarHelper.unpack_from_jar ("eego-SDK.dll");
        } else if (SystemUtils.IS_OS_MAC)
        {
            lib_name = "libBoardController.dylib";
            JarHelper.unpack_from_jar ("libGanglionLib.dylib");
            JarHelper.unpack_from_jar ("libneurosdk-shared.dylib");
            JarHelper.unpack_from_jar ("libsimpleble-c.dylib");
            JarHelper.unpack_from_jar ("libMuseLib.dylib");
            JarHelper.unpack_from_jar ("libBrainBitLib.dylib");
            JarHelper.unpack_from_jar ("libBrainFlowBluetooth.dylib");
        } else if ((SystemUtils.IS_OS_LINUX) && (!is_os_android))
        {
            JarHelper.unpack_from_jar ("libunicorn.so");
            JarHelper.unpack_from_jar ("libGanglionLib.so");
            JarHelper.unpack_from_jar ("libsimpleble-c.so");
            JarHelper.unpack_from_jar ("libMuseLib.so");
            JarHelper.unpack_from_jar ("libBrainFlowBluetooth.so");
            JarHelper.unpack_from_jar ("libBrainBitLib.so");
            JarHelper.unpack_from_jar ("libeego-SDK.so");
        }

        if (is_os_android)
        {
            // for android you need to put these files manually to jniLibs folder, unpacking
            // doesnt work
            lib_name = "BoardController"; // no lib prefix and no extension for android
        } else
        {
            // need to extract libraries from jar
            Path lib_path = JarHelper.unpack_from_jar (lib_name);
            if (lib_path != null)
            {
                lib_name = lib_path.toString ();
            }
        }

        instance = Native.loadLibrary (lib_name, DllInterface.class,
                Collections.singletonMap (Library.OPTION_ALLOW_OBJECTS, Boolean.TRUE));
        instance.java_set_jnienv (JNIEnv.CURRENT);
    }

    /**
     * enable BrainFlow logger with level INFO
     */
    public static void enable_board_logger () throws BrainFlowError
    {
        set_log_level (2);
    }

    /**
     * enable BrainFlow logger with level TRACE
     */
    public static void enable_dev_board_logger () throws BrainFlowError
    {
        set_log_level (0);
    }

    /**
     * disable BrainFlow logger
     */
    public static void disable_board_logger () throws BrainFlowError
    {
        set_log_level (6);
    }

    /**
     * redirect logger from stderr to a file
     */
    public static void set_log_file (String log_file) throws BrainFlowError
    {
        int ec = instance.set_log_file_board_controller (log_file);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_file", ec);
        }
    }

    /**
     * release all prepared sessions
     */
    public static void release_all_sessions () throws BrainFlowError
    {
        int ec = instance.release_all_sessions ();
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in release sessions", ec);
        }
    }

    /**
     * set log level
     */
    public static void set_log_level (int log_level) throws BrainFlowError
    {
        int ec = instance.set_log_level_board_controller (log_level);
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
     * send user defined strings to BrainFlow logger
     */
    public static void log_message (int log_level, String message) throws BrainFlowError
    {
        int ec = instance.log_message_board_controller (log_level, message);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in log_message", ec);
        }
    }

    /**
     * send user defined strings to BrainFlow logger
     */
    public static void log_message (LogLevels log_level, String message) throws BrainFlowError
    {
        log_message (log_level.get_code (), message);
    }

    /**
     * get sampling rate for this board
     */
    public static int get_sampling_rate (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_sampling_rate (board_id, preset.get_code (), res);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get sampling rate for this board
     */
    public static int get_sampling_rate (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_sampling_rate (board_id.get_code (), preset);
    }

    /**
     * get sampling rate for this board
     */
    public static int get_sampling_rate (BoardIds board_id) throws BrainFlowError
    {
        return get_sampling_rate (board_id.get_code (), BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get sampling rate for this board
     */
    public static int get_sampling_rate (int board_id) throws BrainFlowError
    {
        return get_sampling_rate (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains
     * timestamps
     */
    public static int get_timestamp_channel (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_timestamp_channel (board_id, preset.get_code (), res);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains
     * timestamps
     */
    public static int get_timestamp_channel (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_timestamp_channel (board_id.get_code (), preset);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains
     * timestamps
     */
    public static int get_timestamp_channel (BoardIds board_id) throws BrainFlowError
    {
        return get_timestamp_channel (board_id.get_code (), BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains
     * timestamps
     */
    public static int get_timestamp_channel (int board_id) throws BrainFlowError
    {
        return get_timestamp_channel (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains markers
     */
    public static int get_marker_channel (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_marker_channel (board_id, preset.get_code (), res);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains markers
     */
    public static int get_marker_channel (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_marker_channel (board_id.get_code (), preset);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains markers
     */
    public static int get_marker_channel (BoardIds board_id) throws BrainFlowError
    {
        return get_marker_channel (board_id.get_code (), BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains markers
     */
    public static int get_marker_channel (int board_id) throws BrainFlowError
    {
        return get_marker_channel (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get number of rows in returned by get_board_data() 2d array
     */
    public static int get_num_rows (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_num_rows (board_id, preset.get_code (), res);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get number of rows in returned by get_board_data() 2d array
     */
    public static int get_num_rows (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_num_rows (board_id.get_code (), preset);
    }

    /**
     * get number of rows in returned by get_board_data() 2d array
     */
    public static int get_num_rows (BoardIds board_id) throws BrainFlowError
    {
        return get_num_rows (board_id.get_code (), BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get number of rows in returned by get_board_data() 2d array
     */
    public static int get_num_rows (int board_id) throws BrainFlowError
    {
        return get_num_rows (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains package
     * nums
     */
    public static int get_package_num_channel (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_package_num_channel (board_id, preset.get_code (), res);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains package
     * nums
     */
    public static int get_package_num_channel (int board_id) throws BrainFlowError
    {
        return get_package_num_channel (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains package
     * nums
     */
    public static int get_package_num_channel (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_package_num_channel (board_id.get_code (), preset);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains package
     * nums
     */
    public static int get_package_num_channel (BoardIds board_id) throws BrainFlowError
    {
        return get_package_num_channel (board_id.get_code ());
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains battery
     * level
     */
    public static int get_battery_channel (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_battery_channel (board_id, preset.get_code (), res);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains battery
     * level
     */
    public static int get_battery_channel (int board_id) throws BrainFlowError
    {
        return get_battery_channel (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains battery
     * level
     */
    public static int get_battery_channel (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_battery_channel (board_id.get_code (), preset);
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains battery
     * level
     */
    public static int get_battery_channel (BoardIds board_id) throws BrainFlowError
    {
        return get_battery_channel (board_id.get_code ());
    }

    /**
     * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
     * locations
     */
    public static String[] get_eeg_names (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[4096];
        int ec = instance.get_eeg_names (board_id, preset.get_code (), str, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        String eeg_names_string = new String (str, 0, len[0]);
        return eeg_names_string.split (",");
    }

    /**
     * Get supported presets for this device
     */
    public static BrainFlowPresets[] get_board_presets (int board_id) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] presets = new int[512];
        int ec = instance.get_board_presets (board_id, presets, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        BrainFlowPresets[] res = new BrainFlowPresets[len[0]];
        for (int i = 0; i < len[0]; i++)
        {
            res[i] = BrainFlowPresets.from_code (presets[i]);
        }
        return res;

    }

    /**
     * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
     * locations
     */
    public static String[] get_eeg_names (int board_id) throws BrainFlowError
    {
        return get_eeg_names (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
     * locations
     */
    public static String[] get_eeg_names (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_eeg_names (board_id.get_code (), preset);
    }

    /**
     * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
     * locations
     */
    public static String[] get_eeg_names (BoardIds board_id) throws BrainFlowError
    {
        return get_eeg_names (board_id.get_code ());
    }

    /**
     * Get board description
     */
    public static <T> T get_board_descr (Class<T> type, int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[16000];
        int ec = instance.get_board_descr (board_id, preset.get_code (), str, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        String descr_string = new String (str, 0, len[0]);
        Gson gson = new Gson ();
        T res = gson.fromJson (descr_string, type);
        return res;
    }

    /**
     * Get board description
     */
    public static <T> T get_board_descr (Class<T> type, int board_id) throws BrainFlowError
    {
        return get_board_descr (type, board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * Get board description
     */
    public static <T> T get_board_descr (Class<T> type, BoardIds board_id, BrainFlowPresets preset)
            throws BrainFlowError
    {
        return get_board_descr (type, board_id.get_code (), preset);
    }

    /**
     * Get board description
     */
    public static <T> T get_board_descr (Class<T> type, BoardIds board_id) throws BrainFlowError
    {
        return get_board_descr (type, board_id.get_code ());
    }

    /**
     * Get device name
     */
    public static String get_device_name (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[4096];
        int ec = instance.get_device_name (board_id, preset.get_code (), str, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        String name = new String (str, 0, len[0]);
        return name;
    }

    /**
     * Get device name
     */
    public static String get_device_name (int board_id) throws BrainFlowError
    {
        return get_device_name (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * Get device name
     */
    public static String get_device_name (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_device_name (board_id.get_code (), preset);
    }

    /**
     * Get device name
     */
    public static String get_device_name (BoardIds board_id) throws BrainFlowError
    {
        return get_device_name (board_id.get_code ());
    }

    /**
     * Get version
     */
    public static String get_version () throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[64];
        int ec = instance.get_version_board_controller (str, len, 64);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in get_version", ec);
        }
        String version = new String (str, 0, len[0]);
        return version;
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EEG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eeg_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_eeg_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EEG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eeg_channels (int board_id) throws BrainFlowError
    {
        return get_eeg_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EEG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eeg_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_eeg_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EEG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eeg_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_eeg_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EMG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_emg_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_emg_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EMG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_emg_channels (int board_id) throws BrainFlowError
    {
        return get_emg_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EMG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_emg_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_emg_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EMG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_emg_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_emg_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain ECG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ecg_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_ecg_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain ECG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ecg_channels (int board_id) throws BrainFlowError
    {
        return get_ecg_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain ECG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ecg_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_ecg_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain ECG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ecg_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_ecg_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * temperature data
     */
    public static int[] get_temperature_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_temperature_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * temperature data
     */
    public static int[] get_temperature_channels (int board_id) throws BrainFlowError
    {
        return get_temperature_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * temperature data
     */
    public static int[] get_temperature_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_temperature_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * temperature data
     */
    public static int[] get_temperature_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_temperature_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * magnetometer data
     */
    public static int[] get_magnetometer_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_magnetometer_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * magnetometer data
     */
    public static int[] get_magnetometer_channels (int board_id) throws BrainFlowError
    {
        return get_magnetometer_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * magnetometer data
     */
    public static int[] get_magnetometer_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_magnetometer_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * magnetometer data
     */
    public static int[] get_magnetometer_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_magnetometer_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * resistance data
     */
    public static int[] get_resistance_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_resistance_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * resistance data
     */
    public static int[] get_resistance_channels (int board_id) throws BrainFlowError
    {
        return get_resistance_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * resistance data
     */
    public static int[] get_resistance_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_resistance_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain
     * resistance data
     */
    public static int[] get_resistance_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_resistance_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EOG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eog_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_eog_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EOG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eog_channels (int board_id) throws BrainFlowError
    {
        return get_eog_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EOG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eog_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_eog_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EOG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eog_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_eog_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EXG
     * data
     */
    public static int[] get_exg_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_exg_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EXG
     * data
     */
    public static int[] get_exg_channels (int board_id) throws BrainFlowError
    {
        return get_exg_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EXG
     * data
     */
    public static int[] get_exg_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_exg_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EXG
     * data
     */
    public static int[] get_exg_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_exg_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EDA
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eda_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_eda_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EDA
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eda_channels (int board_id) throws BrainFlowError
    {
        return get_eda_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EDA
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eda_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_eda_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EDA
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eda_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_eda_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain PPG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ppg_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_ppg_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain PPG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ppg_channels (int board_id) throws BrainFlowError
    {
        return get_ppg_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain PPG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ppg_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_ppg_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain PPG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_ppg_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_ppg_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain accel
     * data
     */
    public static int[] get_accel_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_accel_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain accel
     * data
     */
    public static int[] get_accel_channels (int board_id) throws BrainFlowError
    {
        return get_accel_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain accel
     * data
     */
    public static int[] get_accel_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_accel_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain accel
     * data
     */
    public static int[] get_accel_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_accel_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain analog
     * data
     */
    public static int[] get_analog_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_analog_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain analog
     * data
     */
    public static int[] get_analog_channels (int board_id) throws BrainFlowError
    {
        return get_analog_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain analog
     * data
     */
    public static int[] get_analog_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_analog_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain analog
     * data
     */
    public static int[] get_analog_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_analog_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain gyro
     * data
     */
    public static int[] get_gyro_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_gyro_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain gyro
     * data
     */
    public static int[] get_gyro_channels (int board_id) throws BrainFlowError
    {
        return get_gyro_channels (board_id);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain gyro
     * data
     */
    public static int[] get_gyro_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_gyro_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain gyro
     * data
     */
    public static int[] get_gyro_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_gyro_channels (board_id.get_code ());
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain other
     * data
     */
    public static int[] get_other_channels (int board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_other_channels (board_id, preset.get_code (), channels, len);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain other
     * data
     */
    public static int[] get_other_channels (int board_id) throws BrainFlowError
    {
        return get_other_channels (board_id, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain other
     * data
     */
    public static int[] get_other_channels (BoardIds board_id, BrainFlowPresets preset) throws BrainFlowError
    {
        return get_other_channels (board_id.get_code (), preset);
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain other
     * data
     */
    public static int[] get_other_channels (BoardIds board_id) throws BrainFlowError
    {
        return get_other_channels (board_id.get_code ());
    }

    /**
     * BrainFlow's board id
     */
    public int board_id;
    private String input_json;
    private int master_board_id;

    /**
     * Create BoardShim object
     */
    public BoardShim (int board_id, BrainFlowInputParams params)
            throws BrainFlowError, IOException, ReflectiveOperationException
    {
        this.board_id = board_id;
        this.master_board_id = board_id;
        if (
            (board_id == BoardIds.STREAMING_BOARD.get_code ()) || (board_id == BoardIds.PLAYBACK_FILE_BOARD.get_code ())
        )
        {
            if (params.get_master_board () == BoardIds.NO_BOARD.get_code ())
            {
                throw new BrainFlowError ("need to set master board attribute in BrainFlowInputParams",
                        BrainFlowExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
            } else
            {
                this.master_board_id = params.get_master_board ();
            }
        }
        this.input_json = params.to_json ();
    }

    /**
     * Create BoardShim object
     */
    public BoardShim (BoardIds board_id, BrainFlowInputParams params)
            throws BrainFlowError, IOException, ReflectiveOperationException
    {
        this.board_id = board_id.get_code ();
        this.master_board_id = board_id.get_code ();
        if (
            (board_id.get_code () == BoardIds.STREAMING_BOARD.get_code ())
                    || (board_id.get_code () == BoardIds.PLAYBACK_FILE_BOARD.get_code ())
        )
        {
            if (params.get_master_board () == BoardIds.NO_BOARD.get_code ())
            {
                throw new BrainFlowError ("need to set master board attribute in BrainFlowInputParams",
                        BrainFlowExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
            } else
            {
                this.master_board_id = params.get_master_board ();
            }
        }
        this.input_json = params.to_json ();
    }

    /**
     * prepare steaming session, allocate resources
     */
    public void prepare_session () throws BrainFlowError
    {
        int ec = instance.prepare_session (board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in prepare_session", ec);
        }
    }

    /**
     * Get Board Id, can be different than provided (playback or streaming board)
     */
    public int get_board_id ()
    {
        return master_board_id;
    }

    /**
     * add streamer
     */
    public void add_streamer (String streamer, int preset) throws BrainFlowError
    {
        int ec = instance.add_streamer (streamer, preset, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in add_streamer", ec);
        }
    }

    /**
     * delete streamer
     */
    public void delete_streamer (String streamer, int preset) throws BrainFlowError
    {
        int ec = instance.delete_streamer (streamer, preset, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in delete_streamer", ec);
        }
    }

    public void add_streamer (String streamer, BrainFlowPresets preset) throws BrainFlowError
    {
        add_streamer (streamer, preset.get_code ());
    }

    public void add_streamer (String streamer) throws BrainFlowError
    {
        add_streamer (streamer, BrainFlowPresets.DEFAULT_PRESET);
    }

    public void delete_streamer (String streamer, BrainFlowPresets preset) throws BrainFlowError
    {
        delete_streamer (streamer, preset.get_code ());
    }

    public void delete_streamer (String streamer) throws BrainFlowError
    {
        delete_streamer (streamer, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * send string to a board, use this method carefully and only if you understand
     * what you are doing
     */
    public String config_board (String config) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[4096];
        int ec = instance.config_board (config, str, len, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in config_board", ec);
        }
        String resp = new String (str, 0, len[0]);
        return resp;
    }

    /**
     * send string to a board, dont use it
     */
    public void config_board_with_bytes (byte[] bytes) throws BrainFlowError
    {
        int ec = instance.config_board_with_bytes (bytes, bytes.length, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in config_board", ec);
        }
    }

    /**
     * start streaming thread, store data in internal ringbuffer and stream them
     * from brainflow at the same time
     *
     * @param buffer_size     size of internal ringbuffer
     *
     * @param streamer_params supported vals: "file://%file_name%:w",
     *                        "file://%file_name%:a",
     *                        "streaming_board://%multicast_group_ip%:%port%". Range
     *                        for multicast addresses is from "224.0.0.0" to
     *                        "239.255.255.255"
     */
    public void start_stream (int buffer_size, String streamer_params) throws BrainFlowError
    {
        int ec = instance.start_stream (buffer_size, streamer_params, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in start_stream", ec);
        }
    }

    /**
     * start streaming thread, store data in internal ringbuffer
     */
    public void start_stream () throws BrainFlowError
    {
        start_stream (450000, "");
    }

    /**
     * start streaming thread, store data in internal ringbuffer
     */
    public void start_stream (int buffer_size) throws BrainFlowError
    {
        start_stream (buffer_size, "");
    }

    /**
     * stop streaming thread
     */
    public void stop_stream () throws BrainFlowError
    {
        int ec = instance.stop_stream (board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in stop_stream", ec);
        }
    }

    /**
     * release all resources
     */
    public void release_session () throws BrainFlowError
    {
        int ec = instance.release_session (board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in release_session", ec);
        }
    }

    /**
     * get number of packages in ringbuffer
     */
    public int get_board_data_count (BrainFlowPresets preset) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_board_data_count (preset.get_code (), res, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in get_board_data_count", ec);
        }
        return res[0];
    }

    /**
     * get number of packages in ringbuffer
     */
    public int get_board_data_count () throws BrainFlowError
    {
        return get_board_data_count (BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * insert marker to data stream
     */
    public void insert_marker (double value, BrainFlowPresets preset) throws BrainFlowError
    {
        int ec = instance.insert_marker (value, preset.get_code (), board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in insert_marker", ec);
        }
    }

    /**
     * insert marker to data stream
     */
    public void insert_marker (double value) throws BrainFlowError
    {
        insert_marker (value, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * check session status
     */
    public boolean is_prepared () throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.is_prepared (res, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in is_prepared", ec);
        }
        return res[0] != 0;
    }

    /**
     * get latest collected data, can return less than "num_samples", doesnt flush
     * it from ringbuffer
     */
    public double[][] get_current_board_data (int num_samples, BrainFlowPresets preset) throws BrainFlowError
    {
        int num_rows = BoardShim.get_num_rows (master_board_id, preset);
        double[] data_arr = new double[num_samples * num_rows];
        int[] current_size = new int[1];
        int ec = instance.get_current_board_data (num_samples, preset.get_code (), data_arr, current_size, board_id,
                input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in get_current_board_data", ec);
        }
        double[][] result = new double[num_rows][];
        for (int i = 0; i < num_rows; i++)
        {
            result[i] = Arrays.copyOfRange (data_arr, (i * current_size[0]), (i + 1) * current_size[0]);
        }
        return result;
    }

    /**
     * get latest collected data, can return less than "num_samples", doesnt flush
     * it from ringbuffer
     */
    public double[][] get_current_board_data (int num_samples) throws BrainFlowError
    {

        return get_current_board_data (num_samples, BrainFlowPresets.DEFAULT_PRESET);
    }

    /**
     * get all data from ringbuffer and flush it
     */
    public double[][] get_board_data (BrainFlowPresets preset) throws BrainFlowError
    {
        int size = get_board_data_count (preset);
        int num_rows = BoardShim.get_num_rows (master_board_id, preset);
        double[] data_arr = new double[size * num_rows];
        int ec = instance.get_board_data (size, preset.get_code (), data_arr, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in get_board_data", ec);
        }
        double[][] result = new double[num_rows][];
        for (int i = 0; i < num_rows; i++)
        {
            result[i] = Arrays.copyOfRange (data_arr, (i * size), (i + 1) * size);
        }
        return result;
    }

    public double[][] get_board_data (int num_datapoints, BrainFlowPresets preset) throws BrainFlowError
    {
        if (num_datapoints < 0)
        {
            throw new BrainFlowError ("data size should be greater than 0",
                    BrainFlowExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        int size = get_board_data_count (preset);
        size = (size >= num_datapoints) ? num_datapoints : size;
        int num_rows = BoardShim.get_num_rows (master_board_id, preset);
        double[] data_arr = new double[size * num_rows];
        int ec = instance.get_board_data (size, preset.get_code (), data_arr, board_id, input_json);
        if (ec != BrainFlowExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in get_board_data", ec);
        }
        double[][] result = new double[num_rows][];
        for (int i = 0; i < num_rows; i++)
        {
            result[i] = Arrays.copyOfRange (data_arr, (i * size), (i + 1) * size);
        }
        return result;
    }

    /**
     * get all data from ringbuffer and flush it
     */
    public double[][] get_board_data () throws BrainFlowError
    {
        BrainFlowPresets preset = BrainFlowPresets.DEFAULT_PRESET;
        return get_board_data (get_board_data_count (preset), preset);
    }

    public double[][] get_board_data (int num_datapoints) throws BrainFlowError
    {
        return get_board_data (num_datapoints, BrainFlowPresets.DEFAULT_PRESET);
    }
}
