package brainflow;

import java.io.File;
import java.io.InputStream;
import java.nio.file.Files;
import java.io.IOException;
import java.util.Arrays;

import org.apache.commons.lang3.SystemUtils;

import com.sun.jna.Library;
import com.sun.jna.Native;

public class BoardShim {

    private interface DllInterface extends Library {
        int prepare_session (int board_id, String port_name);
        int config_board (String config, int board_id, String port_name);
        int start_stream (int buffer_size, int board_id, String port_name);
        int stop_stream (int board_id, String port_name);
        int release_session (int board_id, String port_name);
        int get_current_board_data (int num_samples, double[] data_buf, int[] returned_samples, int board_id, String port_name);
        int get_board_data_count (int[] result, int board_id, String port_name);
        int get_board_data (int data_count, double[] data_buf, int board_id, String port_name);
        int set_log_level (int log_level);
        int set_log_file (String log_file);
        int get_sampling_rate (int board_id, int[] sampling_rate);
        int get_package_num_channel (int board_id, int[] package_num_channel);
        int get_num_rows (int board_id, int[] num_rows);
        int get_timestamp_channel (int board_id, int[] timestamp_channel);
        int get_eeg_channels (int board_id, int[] eeg_channels, int[] len);
        int get_emg_channels (int board_id, int[] emg_channels, int[] len);
        int get_ecg_channels (int board_id, int[] ecg_channels, int[] len);
        int get_eog_channels (int board_id, int[] eog_channels, int[] len);
        int get_eda_channels (int board_id, int[] eda_channels, int[] len);
        int get_ppg_channels (int board_id, int[] ppg_channels, int[] len);
        int get_accel_channels (int board_id, int[] accel_channels, int[] len);
        int get_gyro_channels (int board_id, int[] gyro_channels, int[] len);
        int get_other_channels (int board_id, int[] other_channels, int[] len);
    }
    private static DllInterface instance;

    static {
        String lib_name = "libBoardController.so";
        if (SystemUtils.IS_OS_WINDOWS) {
            lib_name = "BoardController.dll";
            
        }
        else if (SystemUtils.IS_OS_MAC) {
            lib_name = "libBoardController.dylib";
        }

        // need to extract libraries from jar
        unpack_from_jar (lib_name);
        unpack_from_jar ("brainflow_boards.json");
        if (SystemUtils.IS_OS_WINDOWS) {
            unpack_from_jar ("GanglionLib.dll");
            unpack_from_jar ("GanglionLibNative64.dll");
        }
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
    
    public static void enable_board_logger () throws BrainFlowError {
        set_log_level (2);
    }

    public static void enable_dev_board_logger () throws BrainFlowError {
        set_log_level (0);
    }

    public static void disable_board_logger () throws BrainFlowError {
        set_log_level (6);
    }

    public static void set_log_file (String log_file) throws BrainFlowError {
        int ec = instance.set_log_file (log_file);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in set_log_file", ec);
        }
    }
    
    private static void set_log_level (int log_level) throws BrainFlowError {
        int ec = instance.set_log_level (log_level);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in set_log_level", ec);
        }
    }
    
    public static int get_sampling_rate (int board_id) throws BrainFlowError {
    	int[] res = new int[1];
        int ec = instance.get_sampling_rate (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }
    
    public static int get_timestamp_channel (int board_id) throws BrainFlowError {
    	int[] res = new int[1];
        int ec = instance.get_timestamp_channel (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }
    
    public static int get_num_rows (int board_id) throws BrainFlowError {
    	int[] res = new int[1];
        int ec = instance.get_num_rows (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }
    
    public static int get_package_num_channel (int board_id) throws BrainFlowError {
    	int[] res = new int[1];
        int ec = instance.get_package_num_channel (board_id, res);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        return res[0];
    }
    
    public static int[] get_eeg_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_eeg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange (channels, 0, len[0]);
    }
    
    public static int[] get_emg_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_emg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public static int[] get_ecg_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_ecg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public static int[] get_eog_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_eog_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public static int[] get_eda_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_eda_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public static int[] get_ppg_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_ppg_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public static int[] get_accel_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_accel_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public static int[] get_gyro_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_gyro_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public static int[] get_other_channels (int board_id) throws BrainFlowError {
    	int[] len = new int[1];
    	int[] channels = new int[512];
        int ec = instance.get_other_channels (board_id, channels, len);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in board info getter", ec);
        }
        
        return Arrays.copyOfRange(channels, 0, len[0]);
    }
    
    public int board_id;
    public String input_json;
    
    public BoardShim (int board_id, BrainFlowInputParams params) throws BrainFlowError, IOException, ReflectiveOperationException {
        this.board_id = board_id;
        this.input_json = params.to_json();
    }

    public void prepare_session () throws BrainFlowError {
        int ec = instance.prepare_session (board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in prepare_session", ec);
        }
    }

    public void config_board (String config) throws BrainFlowError {
        int ec = instance.config_board (config, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in config_board", ec);
        }
    }

    public void start_stream (int buffer_size) throws BrainFlowError {
        int ec = instance.start_stream (buffer_size, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in start_stream", ec);
        }
    }

    public void stop_stream () throws BrainFlowError {
        int ec = instance.stop_stream (board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in stop_stream", ec);
        }
    }

    public void release_session () throws BrainFlowError {
        int ec = instance.release_session (board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in release_session", ec);
        }
    }

    public int get_board_data_count () throws BrainFlowError {
        int[] res = new int[1];
        int ec = instance.get_board_data_count (res, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in get_board_data_count", ec);
        }
        return res[0];
    }

    public double[][] get_current_board_data (int num_samples) throws BrainFlowError {
    	int num_rows = BoardShim.get_num_rows(board_id);
        double[] data_arr = new double[num_samples * num_rows];
        int[] current_size = new int[1];
        int ec = instance.get_current_board_data (num_samples, data_arr, current_size, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in get_current_board_data", ec);
        }
        double[][] result = new double[num_rows][];
        for (int i = 0; i < num_rows; i++) {
        	result[i] = Arrays.copyOfRange (data_arr, (i * current_size[0]), (i + 1) * current_size[0]);
        }
        return result;
    }

    public double[][] get_board_data () throws BrainFlowError {
        int size = get_board_data_count ();
        int num_rows = BoardShim.get_num_rows(board_id);
        double[] data_arr = new double[size * num_rows];
        int ec = instance.get_board_data (size, data_arr, board_id, input_json);
        if (ec != ExitCode.STATUS_OK.get_code ()) {
            throw new BrainFlowError ("Error in get_board_data", ec);
        }
        double[][] result = new double[num_rows][];
        for (int i = 0; i < num_rows; i++) {
        	result[i] = Arrays.copyOfRange (data_arr, (i * size), (i + 1) * size);
        }
        return result;
    }
}
