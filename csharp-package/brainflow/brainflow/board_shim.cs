using System;
using System.IO;
using System.Runtime.Serialization.Json;
using System.Text;

namespace brainflow
{

    /// <summary>
    /// BoardShim class to communicate with a board
    /// </summary>
    public class BoardShim
    {
        /// <summary>
        /// BrainFlow's board id
        /// </summary>
        public int board_id;
        private string input_json;
        private int master_board_id; // for streaming board

        /// <summary>
        /// Create an instance of BoardShim class
        /// </summary>
        /// <param name="board_id"></param>
        /// <param name="input_params"></param>
        public BoardShim (int board_id, BrainFlowInputParams input_params)
        {
            this.board_id = board_id;
            this.master_board_id = board_id;
            if ((board_id == (int)BoardIds.STREAMING_BOARD) || (board_id == (int)BoardIds.PLAYBACK_FILE_BOARD))
            {
                try
                {
                    master_board_id = System.Convert.ToInt32 (input_params.other_info);
                }
                catch (FormatException)
                {
                    throw new BrainFlowException ((int)CustomExitCodes.INVALID_ARGUMENTS_ERROR);
                }
            }
            this.input_json = input_params.to_json ();
        }

        /// <summary>
        /// get sampling rate for this board id
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>sampling rate</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int get_sampling_rate (int board_id)
        {
            int[] val = new int[1];
            int res = BoardControllerLibrary.get_sampling_rate (board_id, val);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

        /// <summary>
        /// get row index in returned by get_board_data() 2d array which holds package nums
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>row num in 2d array</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int get_package_num_channel (int board_id)
        {
            int[] val = new int[1];
            int res = BoardControllerLibrary.get_package_num_channel (board_id, val);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

        /// <summary>
        /// get row index which holds timestamps
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>row num in 2d array</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int get_timestamp_channel (int board_id)
        {
            int[] val = new int[1];
            int res = BoardControllerLibrary.get_timestamp_channel (board_id, val);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

        /// <summary>
        /// get row index which holds marker
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>row num in 2d array</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int get_marker_channel (int board_id)
        {
            int[] val = new int[1];
            int res = BoardControllerLibrary.get_marker_channel (board_id, val);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

        /// <summary>
        /// get row undex which holds battery level
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>row num in 2d array</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int get_battery_channel (int board_id)
        {
            int[] val = new int[1];
            int res = BoardControllerLibrary.get_battery_channel (board_id, val);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

        /// <summary>
        /// get number of rows in returned by get_board_data() 2d array 
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>number of rows in 2d array</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int get_num_rows (int board_id)
        {
            int[] val = new int[1];
            int res = BoardControllerLibrary.get_num_rows (board_id, val);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            return val[0];
        }

        /// <summary>
        /// get names of EEG channels in 10-20 system. Only if electrodes have fixed locations
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of 10-20 locations</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static string[] get_eeg_names (int board_id)
        {
            int[] len = new int[1];
            byte[] str = new byte[4096];
            int res = BoardControllerLibrary.get_eeg_names (board_id, str, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException(res);
            }
            string eeg_str = System.Text.Encoding.UTF8.GetString (str, 0, len[0]);
            return eeg_str.Split (new Char[] {','});
        }

        /// <summary>
        /// get board description
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>board description</returns>
        /// <exception cref="BrainFlowException">If board id is not valid exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static T get_board_descr<T> (int board_id) where T : class
        {
            int[] len = new int[1];
            byte[] str = new byte[16000];
            int res = BoardControllerLibrary.get_board_descr (board_id, str, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException(res);
            }
            string descr_str = System.Text.Encoding.UTF8.GetString (str, 0, len[0]);
            var ms = new MemoryStream(Encoding.UTF8.GetBytes(descr_str));
            var serializer = new DataContractJsonSerializer(typeof(T));
            var obj = serializer.ReadObject(ms) as T;
            ms.Close();
            return obj;
        }

        /// <summary>
        /// get device name
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>device name</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static string get_device_name (int board_id)
        {
            int[] len = new int[1];
            byte[] str = new byte[4096];
            int res = BoardControllerLibrary.get_device_name (board_id, str, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            string name = System.Text.Encoding.UTF8.GetString (str, 0, len[0]);
            return name;
        }

        /// <summary>
        /// get row indices of EEG channels for this board, for some board we can not split EMG\EEG\.. data and return the same array for all of them
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_eeg_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_eeg_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices of EMG channels for this board, for some board we can not split EMG\EEG\.. data and return the same array for all of them
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_emg_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_emg_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices of ECG channels for this board, for some board we can not split EMG\EEG\.. data and return the same array for all of them
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_ecg_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_ecg_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices of EOG channels for this board, for some board we can not split EMG\EEG\.. data and return the same array for all of them
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_eog_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_eog_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices of EXG channels for this board
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_exg_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_exg_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices of EDA channels for this board, for some board we can not split EMG\EEG\.. data and return the same array for all of them
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_eda_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_eda_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indeces which hold ppg data
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_ppg_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_ppg_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices which hold accel data
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_accel_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_accel_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices which hold analog data 
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_analog_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_analog_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get row indices which hold gyro data
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_gyro_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_gyro_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get other channels for this board
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_other_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_other_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get temperature channels for this board
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_temperature_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_temperature_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// get resistance channels for this board
        /// </summary>
        /// <param name="board_id"></param>
        /// <returns>array of row nums</returns>
        /// <exception cref="BrainFlowException">If this board has no such data exit code is UNSUPPORTED_BOARD_ERROR</exception>
        public static int[] get_resistance_channels (int board_id)
        {
            int[] len = new int[1];
            int[] channels = new int[512];
            int res = BoardControllerLibrary.get_resistance_channels (board_id, channels, len);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException(res);
            }
            int[] result = new int[len[0]];
            for (int i = 0; i < len[0]; i++)
            {
                result[i] = channels[i];
            }
            return result;
        }

        /// <summary>
        /// set log level, logger is disabled by default
        /// </summary>
        /// <param name="log_level"></param>
        public static void set_log_level (int log_level)
        {
            int res = BoardControllerLibrary.set_log_level_board_controller (log_level);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// enable BrainFlow's logger with level INFO
        /// </summary>
        public static void enable_board_logger ()
        {
            set_log_level ((int)LogLevels.LEVEL_INFO);
        }

        /// <summary>
        /// disable BrainFlow's logger
        /// </summary>
        public static void disable_board_logger ()
        {
            set_log_level ((int)LogLevels.LEVEL_OFF);
        }

        /// <summary>
        /// enable BrainFLow's logger with level TRACE
        /// </summary>
        public static void enable_dev_board_logger ()
        {
            set_log_level ((int)LogLevels.LEVEL_TRACE);
        }

        /// <summary>
        /// redirect BrainFlow's logger from stderr to file
        /// </summary>
        /// <param name="log_file"></param>
        public static void set_log_file (string log_file)
        {
            int res = BoardControllerLibrary.set_log_file_board_controller (log_file);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// send your own log message to BrainFlow's logger
        /// </summary>
        /// <param name="log_level"></param>
        /// <param name="message"></param>
        public static void log_message (int log_level, string message)
        {
            int res = BoardControllerLibrary.log_message_board_controller (log_level, message);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// prepare BrainFlow's streaming session, allocate required resources
        /// </summary>
        public void prepare_session ()
        {
            int res = BoardControllerLibrary.prepare_session (board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// send string to a board, use this method carefully and only if you understand what you are doing
        /// </summary>
        /// <param name="config"></param>
        public string config_board (string config)
        {
            int[] len = new int[1];
            byte[] str = new byte[4096];
            int res = BoardControllerLibrary.config_board (config, str, len, board_id, input_json);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
            string response = System.Text.Encoding.UTF8.GetString (str, 0, len[0]);
            return response;
        }

        /// <summary>
        /// insert marker to data array
        /// </summary>
        public void insert_marker (double value)
        {
            int res = BoardControllerLibrary.insert_marker (value, board_id, input_json);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// start streaming thread, store data in internal ringbuffer
        /// </summary>
        /// <param name="buffer_size">size of internal ringbuffer</param>
        /// <param name="streamer_params">supported values: file://%file_name%:w, file://%file_name%:a, streaming_board://%multicast_group_ip%:%port%</param>
        public void start_stream (int buffer_size = 3600 * 250, string streamer_params = "")
        {
            int res = BoardControllerLibrary.start_stream (buffer_size, streamer_params, board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// stop streaming thread, doesnt release other resources
        /// </summary>
        public void stop_stream ()
        {
            int res = BoardControllerLibrary.stop_stream (board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// release BrainFlow's session
        /// </summary>
        public void release_session ()
        {
            int res = BoardControllerLibrary.release_session (board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        /// <summary>
        /// check session status
        /// </summary>
        /// <returns>session status</returns>
        public bool is_prepared ()
        {
            int[] res = new int[1];
            int ec = BoardControllerLibrary.is_prepared(res, board_id, input_json);
            if (ec != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException(ec);
            }
            return res[0] != 0;
        }

        ///<summary>
        /// Get Board Id, for some boards can be different than provided
        ///</summary>
        /// <returns> Master board id </returns>
        public int get_board_id ()
        {
            return master_board_id;
        }
        /// <summary>
        /// get number of packages in ringbuffer
        /// </summary>
        /// <returns>number of packages</returns>
        public int get_board_data_count ()
        {
            int[] res = new int[1];
            int ec = BoardControllerLibrary.get_board_data_count (res, board_id, input_json);
            if (ec != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (ec);
            }
            return res[0];
        }

        /// <summary>
        /// get latest collected data, doesnt remove it from ringbuffer
        /// </summary>
        /// <param name="num_samples"></param>
        /// <returns>latest collected data, can be less than "num_samples"</returns>
        public double[,] get_current_board_data (int num_samples)
        {
            int num_rows = BoardShim.get_num_rows (master_board_id); 
            double[] data_arr = new double[num_samples * num_rows];
            int[] current_size = new int[1];
            int ec = BoardControllerLibrary.get_current_board_data (num_samples, data_arr, current_size, board_id, input_json);
		    if (ec != (int) CustomExitCodes.STATUS_OK) {
			    throw new BrainFlowException (ec);
            }
            double[,] result = new double[num_rows, current_size[0]];
            for (int i = 0; i < num_rows; i++)
            {
                for (int j = 0; j < current_size[0]; j++)
                {
                    result[i, j] = data_arr[i * current_size[0] + j];
                }
            }
            return result;
	    }

        /// <summary>
        /// get all collected data and remove it from ringbuffer
        /// </summary>
        /// <returns>all collected data</returns>
        public double[,] get_board_data ()
        {
            return get_board_data (get_board_data_count ());
        }

        /// <summary>
        /// get collected data and remove it from ringbuffer
        /// </summary>
        /// <returns>all collected data</returns>
        public double[,] get_board_data (int num_datapoints)
        {
            int size = get_board_data_count ();
            if (num_datapoints < 0)
            {
                throw new BrainFlowException ((int)CustomExitCodes.INVALID_ARGUMENTS_ERROR);
            }
            size = Math.Min (size, num_datapoints);
            int num_rows = BoardShim.get_num_rows (master_board_id);
            double[] data_arr = new double[size * num_rows];
            int ec = BoardControllerLibrary.get_board_data (size, data_arr, board_id, input_json);
            if (ec != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (ec);
            }
            double[,] result = new double[num_rows, size];
            for (int i = 0; i < num_rows; i++)
            {
                for (int j = 0; j < size; j++)
                {
                    result[i, j] = data_arr[i * size + j];
                }
            }
            return result;
        }
    }
}
