using System;

using Accord.Math;


namespace brainflow
{

    public class BoardShim
    {
        public int board_id;
        private string input_json;

        public BoardShim (int board_id, BrainFlowInputParams input_params)
        {
            this.board_id = board_id;
            this.input_json = input_params.to_json ();
        }

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

        public static void enable_board_logger ()
        {
            int res = BoardControllerLibrary.set_log_level (2);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public static void disable_board_logger ()
        {
            int res = BoardControllerLibrary.set_log_level (6);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public static void enable_dev_board_logger ()
        {
            int res = BoardControllerLibrary.set_log_level (0);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public static void set_log_file (string log_file)
        {
            int res = BoardControllerLibrary.set_log_file (log_file);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public void prepare_session ()
        {
            int res = BoardControllerLibrary.prepare_session (board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public void config_board (string config)
        {
            int res = BoardControllerLibrary.config_board (config, board_id, input_json);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public void start_stream (int num_samples = 3600 * 250)
        {
            int res = BoardControllerLibrary.start_stream (num_samples, board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public void stop_stream ()
        {
            int res = BoardControllerLibrary.stop_stream (board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

        public void release_session ()
        {
            int res = BoardControllerLibrary.release_session (board_id, input_json);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowException (res);
            }
        }

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

        public double[,] get_current_board_data (int num_samples)
        {
            int num_rows = BoardShim.get_num_rows (board_id); 
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

        public double[,] get_immediate_board_data ()
        {
		        return get_current_board_data (0);
        }

        public double[,] get_board_data ()
        {
		    int size = get_board_data_count ();
            int num_rows = BoardShim.get_num_rows (board_id);
            double[] data_arr = new double[size * num_rows];
            int ec = BoardControllerLibrary.get_board_data (size, data_arr, board_id, input_json);
		    if (ec != (int) CustomExitCodes.STATUS_OK) {
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
