using System.Runtime.InteropServices;

namespace brainflow
{
    public enum LogLevels
    {
        LEVEL_TRACE = 0,
        LEVEL_DEBUG = 1,
        LEVEL_INFO = 2,
        LEVEL_WARN = 3,
        LEVEL_ERROR = 4,
        LEVEL_CRITICAL = 5,
        LEVEL_OFF = 6
    };

    public enum CustomExitCodes
    {
        STATUS_OK = 0,
        PORT_ALREADY_OPEN_ERROR = 1,
        UNABLE_TO_OPEN_PORT_ERROR = 2,
        SET_PORT_ERROR = 3,
        BOARD_WRITE_ERROR = 4,
        INCOMMING_MSG_ERROR = 5,
        INITIAL_MSG_ERROR = 6,
        BOARD_NOT_READY_ERROR = 7,
        STREAM_ALREADY_RUN_ERROR= 8,
        INVALID_BUFFER_SIZE_ERROR = 9,
        STREAM_THREAD_ERROR = 10,
        STREAM_THREAD_IS_NOT_RUNNING = 11,
        EMPTY_BUFFER_ERROR = 12,
        INVALID_ARGUMENTS_ERROR = 13,
        UNSUPPORTED_BOARD_ERROR = 14,
        BOARD_NOT_CREATED_ERROR = 15,
        ANOTHER_BOARD_IS_CREATED_ERROR = 16,
        GENERAL_ERROR = 17,
        SYNC_TIMEOUT_ERROR = 18,
        JSON_NOT_FOUND_ERROR = 19,
        NO_SUCH_DATA_IN_JSON_ERROR = 20
    };

    public enum BoardIds
    {
        STREAMING_BOARD = -2,
        SYNTHETIC_BOARD = -1,
        CYTON_BOARD = 0,
        GANGLION_BOARD = 1,
        CYTON_DAISY_BOARD = 2,
        NOVAXR_BOARD = 3,
        GANGLION_WIFI_BOARD = 4,
        CYTON_WIFI_BOARD = 5,
        CYTON_DAISY_WIFI_BOARD = 6,
        BRAINBIT_BOARD = 7,
        UNICORN_BOARD = 8,
        CALLIBRI_EEG_BOARD = 9,
        CALLIBRI_EMG_BOARD = 10,
        CALLIBRI_ECG_BOARD = 11,
        FASCIA_BOARD = 12
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
        public static extern int start_stream (int buffer_size, string streamer_params, int board_id, string input_json);
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
        public static extern int log_message (int log_level, string message);
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
        public static extern int get_battery_channel (int board_id, int[] battery_channel);
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
        public static extern int get_analog_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_gyro_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_other_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_temperature_channels (int board_id, int[] channels, int[] len);
        [DllImport("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int is_prepared(int[] prepared, int board_id, string input_json);
        [DllImport("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_names(int board_id, byte[] eeg_names, int[] len);
        [DllImport("BoardController.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_resistance_channels(int board_id, int[] channels, int[] len);
    }

    public static class BoardControllerLibrary32
    {
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare_session (int board_id, string input_json);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int start_stream (int buffer_size, string streamer_params, int board_id, string input_json);
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
        public static extern int log_message (int log_level, string message);
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
        public static extern int get_battery_channel (int board_id, int[] battery_channel);
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
        public static extern int get_analog_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_gyro_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_other_channels (int board_id, int[] channels, int[] len);
        [DllImport ("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_temperature_channels (int board_id, int[] channels, int[] len);
        [DllImport("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int is_prepared(int[] prepared, int board_id, string input_json);
        [DllImport("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_names(int board_id, byte[] eeg_names, int[] len);
        [DllImport("BoardController32.dll", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_resistance_channels(int board_id, int[] channels, int[] len);
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

        public static int start_stream (int buffer_size, string streamer_params, int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.start_stream (buffer_size, streamer_params, board_id, input_json);
            else
                return BoardControllerLibrary32.start_stream (buffer_size, streamer_params, board_id, input_json);
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

        public static int is_prepared(int[] result, int board_id, string input_json)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.is_prepared(result, board_id, input_json);
            else
                return BoardControllerLibrary32.is_prepared(result, board_id, input_json);
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

        public static int log_message (int log_level, string message)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.log_message (log_level, message);
            else
                return BoardControllerLibrary32.log_message (log_level, message);
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

        public static int get_battery_channel (int board_id, int[] battery)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_battery_channel (board_id, battery);
            else
                return BoardControllerLibrary32.get_battery_channel (board_id, battery);
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

        public static int get_eeg_names(int board_id, byte[] names, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_eeg_names(board_id, names, len);
            else
                return BoardControllerLibrary32.get_eeg_names(board_id, names, len);
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

        public static int get_analog_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_analog_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_analog_channels (board_id, channels, len);
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

        public static int get_temperature_channels (int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_temperature_channels (board_id, channels, len);
            else
                return BoardControllerLibrary32.get_temperature_channels (board_id, channels, len);
        }

        public static int get_resistance_channels(int board_id, int[] channels, int[] len)
        {
            if (System.Environment.Is64BitProcess)
                return BoardControllerLibrary64.get_resistance_channels(board_id, channels, len);
            else
                return BoardControllerLibrary32.get_resistance_channels(board_id, channels, len);
        }
    }
}
