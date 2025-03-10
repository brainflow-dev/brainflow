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

    public enum IpProtocolTypes
    {
        NO_IP_PROTOCOL = 0,
        UDP = 1,
        TCP = 2
    };

    public enum BrainFlowPresets
    {
        DEFAULT_PRESET = 0,
        AUXILIARY_PRESET = 1,
        ANCILLARY_PRESET = 2
    };

    public enum BrainFlowExitCodes
    {
        STATUS_OK = 0,
        PORT_ALREADY_OPEN_ERROR = 1,
        UNABLE_TO_OPEN_PORT_ERROR = 2,
        SET_PORT_ERROR = 3,
        BOARD_WRITE_ERROR = 4,
        INCOMMING_MSG_ERROR = 5,
        INITIAL_MSG_ERROR = 6,
        BOARD_NOT_READY_ERROR = 7,
        STREAM_ALREADY_RUN_ERROR = 8,
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
        NO_SUCH_DATA_IN_JSON_ERROR = 20,
        CLASSIFIER_IS_NOT_PREPARED_ERROR = 21,
        ANOTHER_CLASSIFIER_IS_PREPARED_ERROR = 22,
        UNSUPPORTED_CLASSIFIER_AND_METRIC_COMBINATION_ERROR = 23
    };

    public enum BoardIds
    {
        NO_BOARD = -100,
        PLAYBACK_FILE_BOARD = -3,
        STREAMING_BOARD = -2,
        SYNTHETIC_BOARD = -1,
        CYTON_BOARD = 0,
        GANGLION_BOARD = 1,
        CYTON_DAISY_BOARD = 2,
        GALEA_BOARD = 3,
        GANGLION_WIFI_BOARD = 4,
        CYTON_WIFI_BOARD = 5,
        CYTON_DAISY_WIFI_BOARD = 6,
        BRAINBIT_BOARD = 7,
        UNICORN_BOARD = 8,
        CALLIBRI_EEG_BOARD = 9,
        CALLIBRI_EMG_BOARD = 10,
        CALLIBRI_ECG_BOARD = 11,
        NOTION_1_BOARD = 13,
        NOTION_2_BOARD = 14,
        GFORCE_PRO_BOARD = 16,
        FREEEEG32_BOARD = 17,
        BRAINBIT_BLED_BOARD = 18,
        GFORCE_DUAL_BOARD = 19,
        GALEA_SERIAL_BOARD = 20,
        MUSE_S_BLED_BOARD = 21,
        MUSE_2_BLED_BOARD = 22,
        CROWN_BOARD = 23,
        ANT_NEURO_EE_410_BOARD = 24,
        ANT_NEURO_EE_411_BOARD = 25,
        ANT_NEURO_EE_430_BOARD = 26,
        ANT_NEURO_EE_211_BOARD = 27,
        ANT_NEURO_EE_212_BOARD = 28,
        ANT_NEURO_EE_213_BOARD = 29,
        ANT_NEURO_EE_214_BOARD = 30,
        ANT_NEURO_EE_215_BOARD = 31,
        ANT_NEURO_EE_221_BOARD = 32,
        ANT_NEURO_EE_222_BOARD = 33,
        ANT_NEURO_EE_223_BOARD = 34,
        ANT_NEURO_EE_224_BOARD = 35,
        ANT_NEURO_EE_225_BOARD = 36,
        ENOPHONE_BOARD = 37,
        MUSE_2_BOARD = 38,
        MUSE_S_BOARD = 39,
        BRAINALIVE_BOARD = 40,
        MUSE_2016_BOARD = 41,
        MUSE_2016_BLED_BOARD = 42,
        EXPLORE_4_CHAN_BOARD = 44,
        EXPLORE_8_CHAN_BOARD = 45,
        GANGLION_NATIVE_BOARD = 46,
        EMOTIBIT_BOARD = 47,
        GALEA_BOARD_V4 = 48,
        GALEA_SERIAL_BOARD_V4 = 49,
        NTL_WIFI_BOARD = 50,
        ANT_NEURO_EE_511_BOARD = 51,
        FREEEEG128_BOARD = 52,
        AAVAA_V3_BOARD = 53,
        EXPLORE_PLUS_8_CHAN_BOARD = 54,
        EXPLORE_PLUS_32_CHAN_BOARD = 55,
        PIEEG_BOARD = 56,   
        NEUROPAWN_KNIGHT_BOARD = 57,
        SYNCHRONI_TRIO_3_CHANNELS_BOARD = 58,
        SYNCHRONI_OCTO_8_CHANNELS_BOARD = 59,
        OB5000_8_CHANNELS_BOARD = 60,
        SYNCHRONI_PENTO_8_CHANNELS_BOARD = 61,
        SYNCHRONI_UNO_1_CHANNELS_BOARD = 62,
        OB3000_24_CHANNELS_BOARD = 63,
        BIOLISTENER_BOARD = 64,
    };


    // lib name is const in DllImport directive, so I can not use only one class
    // alternative solutios: call loadlibrary from kernel32.dll
    // or rename libs to have the same name and load them from different folders but I dont want to fix it in all bindings
    // so there is no really good solution
    public static class BoardControllerLibrary64
    {
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare_session (int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int start_stream (int buffer_size, string streamer_params, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int stop_stream (int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_session (int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_current_board_data (int num_samples, int preset, [In, Out][MarshalAs (UnmanagedType.LPArray, ArraySubType = UnmanagedType.R8)] double[,] data_buf, int[] returned_samples, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data_count (int preset, int[] result, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data (int data_count, int preset, [In, Out][MarshalAs (UnmanagedType.LPArray, ArraySubType = UnmanagedType.R8)] double[,] data_buf, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_board_controller (int log_level);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int log_message_board_controller (int log_level, string message);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int config_board (string config, byte[] response, int[] len, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int config_board_with_bytes (byte[] bytes, int len, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_board_controller (string log_file);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_sampling_rate (int board_id, int preset, int[] sampling_rate);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_timestamp_channel (int board_id, int preset, int[] timestamp_channel);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_marker_channel (int board_id, int preset, int[] marker_channel);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_package_num_channel (int board_id, int preset, int[] package_num_channel);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_battery_channel (int board_id, int preset, int[] battery_channel);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_rows (int board_id, int preset, int[] num_rows);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_emg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ecg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eog_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eda_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ppg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_accel_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_rotation_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_analog_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_gyro_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_other_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_temperature_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int is_prepared (int[] prepared, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_names (int board_id, int preset, byte[] eeg_names, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_resistance_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_magnetometer_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_exg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_device_name (int board_id, int preset, byte[] name, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int insert_marker (double value, int preset, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_descr (int board_id, int preset, byte[] board_descr, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_all_sessions ();
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_version_board_controller (byte[] version, int[] len, int max_len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_presets (int board_id, int[] names, int[] len);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int add_streamer (string streamer, int preset, int board_id, string input_json);
        [DllImport ("BoardController", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int delete_streamer (string streamer, int preset, int board_id, string input_json);
    }

    public static class BoardControllerLibrary32
    {
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int prepare_session (int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int start_stream (int buffer_size, string streamer_params, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int stop_stream (int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_session (int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_current_board_data (int num_samples, int preset, [In, Out][MarshalAs (UnmanagedType.LPArray, ArraySubType = UnmanagedType.R8)] double[,] data_buf, int[] returned_samples, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data_count (int preset, int[] result, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_data (int data_count, int preset, [In, Out][MarshalAs (UnmanagedType.LPArray, ArraySubType = UnmanagedType.R8)] double[,] data_buf, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_level_board_controller (int log_level);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int log_message_board_controller (int log_level, string message);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int config_board (string config, byte[] response, int[] len, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int config_board_with_bytes (byte[] bytes, int len, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int set_log_file_board_controller (string log_file);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_sampling_rate (int board_id, int preset, int[] sampling_rate);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_timestamp_channel (int board_id, int preset, int[] timestamp_channel);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_marker_channel (int board_id, int preset, int[] marker_channel);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_package_num_channel (int board_id, int preset, int[] package_num_channel);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_battery_channel (int board_id, int preset, int[] battery_channel);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_num_rows (int board_id, int preset, int[] num_rows);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_emg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ecg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eog_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eda_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_ppg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_accel_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_rotation_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_analog_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_gyro_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_other_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_temperature_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int is_prepared (int[] prepared, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_eeg_names (int board_id, int preset, byte[] eeg_names, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_resistance_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_exg_channels (int board_id, int preset, int[] channels, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_device_name (int board_id, int preset, byte[] name, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int insert_marker (double value, int preset, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_descr (int board_id, int preset, byte[] board_descr, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int release_all_sessions ();
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_version_board_controller (byte[] version, int[] len, int max_len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_board_presets (int board_id, int[] names, int[] len);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int add_streamer (string streamer, int preset, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int delete_streamer (string streamer, int preset, int board_id, string input_json);
        [DllImport ("BoardController32", SetLastError = true, CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_magnetometer_channels (int board_id, int preset, int[] channels, int[] len);
    }


    public static class BoardControllerLibrary
    {
        public static int prepare_session (int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.prepare_session (board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.prepare_session (board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int start_stream (int buffer_size, string streamer_params, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.start_stream (buffer_size, streamer_params, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.start_stream (buffer_size, streamer_params, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int stop_stream (int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.stop_stream (board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.stop_stream (board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int release_session (int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.release_session (board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.release_session (board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_current_board_data (int num_samples, int preset, [In, Out][MarshalAs (UnmanagedType.LPArray, ArraySubType = UnmanagedType.R8)] double[,] data_buf, int[] returned_samples, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_current_board_data (num_samples, preset, data_buf, returned_samples, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_current_board_data (num_samples, preset, data_buf, returned_samples, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_board_data_count (int preset, int[] result, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_board_data_count (preset, result, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_board_data_count (preset, result, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int is_prepared (int[] result, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.is_prepared (result, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.is_prepared (result, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_board_data (int data_count, int preset, [In, Out][MarshalAs (UnmanagedType.LPArray, ArraySubType = UnmanagedType.R8)] double[,] data_buf, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_board_data (data_count, preset, data_buf, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_board_data (data_count, preset, data_buf, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int set_log_level_board_controller (int log_level)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.set_log_level_board_controller (log_level);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.set_log_level_board_controller (log_level);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int log_message_board_controller (int log_level, string message)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.log_message_board_controller (log_level, message);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.log_message_board_controller (log_level, message);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int insert_marker (double value, int preset, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.insert_marker (value, preset, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.insert_marker (value, preset, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }


        public static int config_board (string config, byte[] str, int[] len, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.config_board (config, str, len, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.config_board (config, str, len, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int config_board_with_bytes (byte[] bytes, int len, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.config_board_with_bytes (bytes, len, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.config_board_with_bytes (bytes, len, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int add_streamer (string streamer, int preset, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.add_streamer (streamer, preset, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.add_streamer (streamer, preset, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int delete_streamer (string streamer, int preset, int board_id, string input_json)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.delete_streamer (streamer, preset, board_id, input_json);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.delete_streamer (streamer, preset, board_id, input_json);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int set_log_file_board_controller (string log_file)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.set_log_file_board_controller (log_file);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.set_log_file_board_controller (log_file);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_sampling_rate (int board_id, int preset, int[] sampling_rate)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_sampling_rate (board_id, preset, sampling_rate);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_sampling_rate (board_id, preset, sampling_rate);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_package_num_channel (int board_id, int preset, int[] package_num)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_package_num_channel (board_id, preset, package_num);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_package_num_channel (board_id, preset, package_num);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_battery_channel (int board_id, int preset, int[] battery)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_battery_channel (board_id, preset, battery);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_battery_channel (board_id, preset, battery);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_num_rows (int board_id, int preset, int[] num_rows)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_num_rows (board_id, preset, num_rows);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_num_rows (board_id, preset, num_rows);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_timestamp_channel (int board_id, int preset, int[] timestamp_channel)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_timestamp_channel (board_id, preset, timestamp_channel);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_timestamp_channel (board_id, preset, timestamp_channel);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_marker_channel (int board_id, int preset, int[] marker_channel)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_marker_channel (board_id, preset, marker_channel);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_marker_channel (board_id, preset, marker_channel);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_eeg_names (int board_id, int preset, byte[] names, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_eeg_names (board_id, preset, names, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_eeg_names (board_id, preset, names, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_board_presets (int board_id, int[] names, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_board_presets (board_id, names, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_board_presets (board_id, names, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_board_descr (int board_id, int preset, byte[] descr, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_board_descr (board_id, preset, descr, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_board_descr (board_id, preset, descr, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_device_name (int board_id, int preset, byte[] name, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_device_name (board_id, preset, name, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_device_name (board_id, preset, name, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_version_board_controller (byte[] version, int[] len, int max_len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_version_board_controller (version, len, max_len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_version_board_controller (version, len, max_len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_eeg_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_eeg_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_eeg_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_exg_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_exg_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_exg_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_emg_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_emg_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_emg_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_ecg_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_ecg_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_ecg_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_eog_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_eog_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_eog_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_eda_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_eda_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_eda_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_ppg_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_ppg_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_ppg_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_accel_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_accel_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_accel_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_rotation_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_rotation_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_rotation_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_analog_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_analog_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_analog_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_gyro_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_gyro_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_gyro_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_other_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_other_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_other_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_temperature_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_temperature_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_temperature_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_resistance_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_resistance_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_resistance_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int get_magnetometer_channels (int board_id, int preset, int[] channels, int[] len)
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.get_magnetometer_channels (board_id, preset, channels, len);
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.get_magnetometer_channels (board_id, preset, channels, len);
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }

        public static int release_all_sessions ()
        {
            switch (PlatformHelper.get_library_environment ())
            {
                case LibraryEnvironment.x64:
                    return BoardControllerLibrary64.release_all_sessions ();
                case LibraryEnvironment.x86:
                    return BoardControllerLibrary32.release_all_sessions ();
            }

            return (int)BrainFlowExitCodes.GENERAL_ERROR;
        }
    }
}
