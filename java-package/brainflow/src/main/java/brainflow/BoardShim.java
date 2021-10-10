package brainflow;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Files;
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
public class BoardShim
{

    private interface DllInterface extends Library
    {
        int prepare_session (int board_id, String params);

        int config_board (String config, byte[] names, int[] len, int board_id, String params);

        int start_stream (int buffer_size, String streamer_params, int board_id, String params);

        int stop_stream (int board_id, String params);

        int release_session (int board_id, String params);

        int insert_marker (double value, int board_id, String params);

        int get_current_board_data (int num_samples, double[] data_buf, int[] returned_samples, int board_id,
                String params);

        int get_board_data_count (int[] result, int board_id, String params);

        int get_board_data (int data_count, double[] data_buf, int board_id, String params);

        int set_log_level_board_controller (int log_level);

        int log_message_board_controller (int log_level, String message);

        int set_log_file_board_controller (String log_file);

        int java_set_jnienv (JNIEnv java_jnienv);

        int get_sampling_rate (int board_id, int[] sampling_rate);

        int get_battery_channel (int board_id, int[] battery_channel);

        int get_package_num_channel (int board_id, int[] package_num_channel);

        int get_num_rows (int board_id, int[] num_rows);

        int get_timestamp_channel (int board_id, int[] timestamp_channel);

        int get_marker_channel (int board_id, int[] marker_channel);

        int get_eeg_channels (int board_id, int[] eeg_channels, int[] len);

        int get_exg_channels (int board_id, int[] eeg_channels, int[] len);

        int get_emg_channels (int board_id, int[] emg_channels, int[] len);

        int get_ecg_channels (int board_id, int[] ecg_channels, int[] len);

        int get_eog_channels (int board_id, int[] eog_channels, int[] len);

        int get_eda_channels (int board_id, int[] eda_channels, int[] len);

        int get_ppg_channels (int board_id, int[] ppg_channels, int[] len);

        int get_accel_channels (int board_id, int[] accel_channels, int[] len);

        int get_analog_channels (int board_id, int[] analog_channels, int[] len);

        int get_gyro_channels (int board_id, int[] gyro_channels, int[] len);

        int get_other_channels (int board_id, int[] other_channels, int[] len);

        int get_resistance_channels (int board_id, int[] channels, int[] len);

        int get_temperature_channels (int board_id, int[] temperature_channels, int[] len);

        int is_prepared (int[] prepared, int board_id, String params);

        int get_eeg_names (int board_id, byte[] names, int[] len);

        int get_board_descr (int board_id, byte[] names, int[] len);

        int get_device_name (int board_id, byte[] name, int[] len);
    }

    private static DllInterface instance;

    static
    {
        // SystemUtils doesnt have smth like IS_OS_ANDROID, need to check using
        // properties
        boolean is_os_android = "The Android Project".equals (System.getProperty ("java.specification.vendor"));

        String lib_name = "libBoardController.so";
        String ganglion_name = "libGanglionLib.so";
        if (SystemUtils.IS_OS_WINDOWS)
        {
            lib_name = "BoardController.dll";
            ganglion_name = "GanglionLib.dll";
            unpack_from_jar ("neurosdk-x64.dll");
            unpack_from_jar ("Unicorn.dll");
            unpack_from_jar ("gForceSDKWrapper.dll");
            unpack_from_jar ("gforce64.dll");
        } else if (SystemUtils.IS_OS_MAC)
        {
            lib_name = "libBoardController.dylib";
            ganglion_name = "libGanglionLib.dylib";
            unpack_from_jar ("libneurosdk-shared.dylib");
        } else if ((SystemUtils.IS_OS_LINUX) && (!is_os_android))
        {
            unpack_from_jar ("libunicorn.so");
        }

        if (is_os_android)
        {
            // for android you need to put these files manually to jniLibs folder, unpacking
            // doesnt work
            lib_name = "BoardController"; // no lib prefix and no extension for android
        } else
        {
            // need to extract libraries from jar
            unpack_from_jar (lib_name);
            unpack_from_jar (ganglion_name);
        }

        instance = Native.loadLibrary (lib_name, DllInterface.class,
                Collections.singletonMap (Library.OPTION_ALLOW_OBJECTS, Boolean.TRUE));
        instance.java_set_jnienv (JNIEnv.CURRENT);
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
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_file", ec);
        }
    }

    /**
     * set log level
     */
    public static void set_log_level (int log_level) throws BrainFlowError
    {
        int ec = instance.set_log_level_board_controller (log_level);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in set_log_level", ec);
        }
    }

    /**
     * send user defined strings to BrainFlow logger
     */
    public static void log_message (int log_level, String message) throws BrainFlowError
    {
        int ec = instance.log_message_board_controller (log_level, message);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in log_message", ec);
        }
    }

    /**
     * get sampling rate for this board
     */
    public static int get_sampling_rate (int board_id) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_sampling_rate (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains
     * timestamps
     */
    public static int get_timestamp_channel (int board_id) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_timestamp_channel (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get row index in returned by get_board_data() 2d array which contains markers
     */
    public static int get_marker_channel (int board_id) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_marker_channel (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * get number of rows in returned by get_board_data() 2d array
     */
    public static int get_num_rows (int board_id) throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_num_rows (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] res = new int[1];
        int ec = instance.get_package_num_channel (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] res = new int[1];
        int ec = instance.get_battery_channel (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }

    /**
     * Get names of EEG electrodes in 10-20 system. Only if electrodes have freezed
     * locations
     */
    public static String[] get_eeg_names (int board_id) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[4096];
        int ec = instance.get_eeg_names (board_id, str, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        String eeg_names_string = new String (str, 0, len[0]);
        return eeg_names_string.split (",");
    }

    /**
     * Get board description
     */
    public static <T> T get_board_descr (Class<T> type, int board_id) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[16000];
        int ec = instance.get_board_descr (board_id, str, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        String descr_string = new String (str, 0, len[0]);
        Gson gson = new Gson ();
        T res = gson.fromJson (descr_string, type);
        return res;
    }

    /**
     * Get device name
     */
    public static String get_device_name (int board_id) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[4096];
        int ec = instance.get_device_name (board_id, str, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        String name = new String (str, 0, len[0]);
        return name;
    }

    /**
     * get row indices in returned by get_board_data() 2d array which contain EEG
     * data, for some boards we can not split EEG\EMG\... and return the same array
     */
    public static int[] get_eeg_channels (int board_id) throws BrainFlowError
    {
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_eeg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_emg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_ecg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_temperature_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_resistance_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_eog_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_exg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_eda_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_ppg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_accel_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_analog_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_gyro_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        int[] len = new int[1];
        int[] channels = new int[512];
        int ec = instance.get_other_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in board info getter", ec);
        }

        return Arrays.copyOfRange (channels, 0, len[0]);
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
            try
            {
                this.master_board_id = Integer.parseInt (params.other_info);
            } catch (NumberFormatException e)
            {
                throw new BrainFlowError ("need to set params.other_info to master board id",
                        ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
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
        if (ec != ExitCode.STATUS_OK.get_code ())
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
     * send string to a board, use this method carefully and only if you understand
     * what you are doing
     */
    public String config_board (String config) throws BrainFlowError
    {
        int[] len = new int[1];
        byte[] str = new byte[4096];
        int ec = instance.config_board (config, str, len, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in config_board", ec);
        }
        String resp = new String (str, 0, len[0]);
        return resp;
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
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        if (ec != ExitCode.STATUS_OK.get_code ())
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
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in release_session", ec);
        }
    }

    /**
     * get number of packages in ringbuffer
     */
    public int get_board_data_count () throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.get_board_data_count (res, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in get_board_data_count", ec);
        }
        return res[0];
    }

    /**
     * insert marker to data stream
     */
    public void insert_marker (double value) throws BrainFlowError
    {
        int ec = instance.insert_marker (value, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in insert_marker", ec);
        }
    }

    /**
     * check session status
     */
    public boolean is_prepared () throws BrainFlowError
    {
        int[] res = new int[1];
        int ec = instance.is_prepared (res, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ())
        {
            throw new BrainFlowError ("Error in is_prepared", ec);
        }
        return res[0] != 0;
    }

    /**
     * get latest collected data, can return less than "num_samples", doesnt flush
     * it from ringbuffer
     */
    public double[][] get_current_board_data (int num_samples) throws BrainFlowError
    {
        int num_rows = BoardShim.get_num_rows (master_board_id);
        double[] data_arr = new double[num_samples * num_rows];
        int[] current_size = new int[1];
        int ec = instance.get_current_board_data (num_samples, data_arr, current_size, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
     * get all data from ringbuffer and flush it
     */
    public double[][] get_board_data () throws BrainFlowError
    {
        int size = get_board_data_count ();
        int num_rows = BoardShim.get_num_rows (master_board_id);
        double[] data_arr = new double[size * num_rows];
        int ec = instance.get_board_data (size, data_arr, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ())
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

    public double[][] get_board_data (int num_datapoints) throws BrainFlowError
    {
        if (num_datapoints < 0)
        {
            throw new BrainFlowError ("data size should be greater than 0",
                    ExitCode.INVALID_ARGUMENTS_ERROR.get_code ());
        }
        int size = get_board_data_count ();
        size = (size >= num_datapoints) ? num_datapoints : size;
        int num_rows = BoardShim.get_num_rows (master_board_id);
        double[] data_arr = new double[size * num_rows];
        int ec = instance.get_board_data (size, data_arr, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ())
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
}
