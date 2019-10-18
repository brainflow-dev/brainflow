using System.Runtime.InteropServices;

namespace brainflow
{
    public enum CustomExitCodes
    {
        STATUS_OK = 0,
        PORT_ALREADY_OPEN_ERROR,
        UNABLE_TO_OPEN_PORT_ERROR,
        SET_PORT_ERROR,
        BOARD_WRITE_ERROR,
        INCOMMING_MSG_ERROR,
        INITIAL_MSG_ERROR,
        BOARD_NOT_READY_ERROR,
        STREAM_ALREADY_RUN_ERROR,
        INVALID_BUFFER_SIZE_ERROR,
        STREAM_THREAD_ERROR,
        STREAM_THREAD_IS_NOT_RUNNING,
        EMPTY_BUFFER_ERROR,
        INVALID_ARGUMENTS_ERROR,
        UNSUPPORTED_BOARD_ERROR,
        BOARD_NOT_CREATED_ERROR,
        ANOTHER_BOARD_IS_CREATED_ERROR,
        GENERAL_ERROR,
        SYNC_TIMEOUT_ERROR,
        JSON_NOT_FOUND_ERROR,
        NO_SUCH_DATA_IN_JSON_ERROR
    };

    public enum BoardIds
    {
        SYNTHETIC_BOARD = -1,
        CYTON_BOARD = 0,
        GANGLION_BOARD = 1,
        CYTON_DAISY_BOARD = 2,
        NOVAXR_BOARD = 3,
        GANGLION_WIFI = 4,
        CYTON_WIFI = 5,
        CYTON_DAISY_WIFI = 6
    };


    // lib name is const in DllImport directive, so I can not use only one class
    // alternative solutios: call loadBoardController from kernel32.dll
    // or rename libs to have the same name and load them from different folders but I dont want to fix it in all bindings
    // so there is no really good solution
    public static class BoardControllerLibrary64
    {
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare_session (int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int start_stream (int buffer_size, int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int stop_stream (int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_session (int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_current_board_data (int num_samples, double[] data_buf, int[] returned_samples, int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data_count(int[] result, int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data (int data_count, double[] data_buf, int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level (int log_level);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int config_board (string config, int board_id, string input_json);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file (string log_file);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_sampling_rate (int board_id, int[] sampling_rate);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_timestamp_channel (int board_id, int[] timestamp_channel);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_package_num_channel (int board_id, int[] package_num_channel);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_rows (int board_id, int[] num_rows);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_emg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ecg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eog_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eda_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ppg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_accel_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_gyro_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_other_channels (int board_id, int[] channels, int[] len);
    }

    public static class BoardControllerLibrary32
    {
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare_session (int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int start_stream (int buffer_size, int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int stop_stream (int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_session (int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_current_board_data (int num_samples, double[] data_buf, int[] returned_samples, int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data_count (int[] result, int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data (int data_count, double[] data_buf, int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level (int log_level);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int config_board (string config, int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file (string log_file);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_sampling_rate (int board_id, int[] sampling_rate);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_timestamp_channel (int board_id, int[] timestamp_channel);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_package_num_channel (int board_id, int[] package_num_channel);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_rows (int board_id, int[] num_rows);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_emg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ecg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eog_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eda_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ppg_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_accel_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_gyro_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_other_channels (int board_id, int[] channels, int[] len);
    }

    public static class BoardControllerLibrary
    {
        public static int prepare_session (int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.prepare_session (board_id, input_json);
            else
                return BoardControllerLibrary32.prepare_session (board_id, input_json);
        }

        public static int start_stream (int buffer_size, int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.start_stream (buffer_size, board_id, input_json);
            else
                return BoardControllerLibrary32.start_stream (buffer_size, board_id, input_json);
        }

        public static int stop_stream (int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.stop_stream (board_id, input_json);
            else
                return BoardControllerLibrary32.stop_stream (board_id, input_json);
        }

        public static int release_session (int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.release_session (board_id, input_json);
            else
                return BoardControllerLibrary32.release_session (board_id, input_json);
        }

        public static int get_current_board_data (int num_samples, double[] data_buf, int[] returned_samples, int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_current_board_data (num_samples, data_buf, returned_samples, board_id, input_json);
            else
                return BoardControllerLibrary32.get_current_board_data (num_samples, data_buf, returned_samples, board_id, input_json);
        }

        public static int get_board_data_count (int[] result, int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_board_data_count (result, board_id, input_json);
            else
                return BoardControllerLibrary32.get_board_data_count (result, board_id, input_json);
        }

        public static int get_board_data (int data_count, double[] data_buf, int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_board_data (data_count, data_buf, board_id, input_json);
            else
                return BoardControllerLibrary32.get_board_data (data_count, data_buf, board_id, input_json);
        }

        public static int set_log_level (int log_level)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.set_log_level (log_level);
            else
                return BoardControllerLibrary32.set_log_level (log_level);
        }

        public static int config_board (string config, int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.config_board (config, board_id, input_json);
            else
                return BoardControllerLibrary32.config_board (config, board_id, input_json);
        }

        public static int set_log_file (string log_file)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.set_log_file (log_file);
            else
                return BoardControllerLibrary32.set_log_file (log_file);
        }
   
        public static int get_sampling_rate (int board_id, int[] sampling_rate)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_sampling_rate (board_id, sampling_rate);
            else
                return BoardControllerLibrary32.get_sampling_rate (board_id, sampling_rate);
        }

        public static int get_package_num_channel (int board_id, int[] package_num)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_package_num_channel (board_id, package_num);
            else
                return BoardControllerLibrary32.get_package_num_channel (board_id, package_num);
        }

        public static int get_num_rows (int board_id, int[] num_rows)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_num_rows (board_id, num_rows);
            else
                return BoardControllerLibrary32.get_num_rows (board_id, num_rows);
        }

        public static int get_timestamp_channel (int board_id, int[] timestamp_channel)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_timestamp_channel (board_id, timestamp_channel);
            else
                return BoardControllerLibrary32.get_timestamp_channel (board_id, timestamp_channel);
        }

        public static int get_eeg_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_eeg_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_eeg_channels (board_id, channels, len);
        }

        public static int get_emg_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_emg_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_emg_channels (board_id, channels, len);
        }

        public static int get_ecg_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_ecg_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_ecg_channels (board_id, channels, len);
        }

        public static int get_eog_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_eog_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_eog_channels (board_id, channels, len);
        }

        public static int get_eda_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_eda_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_eda_channels (board_id, channels, len);
        }

        public static int get_ppg_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_ppg_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_ppg_channels (board_id, channels, len);
        }

        public static int get_accel_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_accel_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_accel_channels (board_id, channels, len);
        }

        public static int get_gyro_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_gyro_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_gyro_channels (board_id, channels, len);
        }

        public static int get_other_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_other_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_other_channels (board_id, channels, len);
        }
    }
}
