using System;

namespace brainflow
{

    public class BoardShim
    {
        public int board_id;
        public string port_name;
        public int fs_hz;
        public int num_eeg_channels;
        public int first_eeg_channel;
        public int package_length;

        public BoardShim (int board_id, string port_name)
        {
            this.board_id = board_id;
            this.port_name = port_name;
            this.package_length = BoardInfoGetter.get_package_length (board_id);
            this.fs_hz = BoardInfoGetter.get_fs_hz (board_id);
            this.num_eeg_channels = BoardInfoGetter.get_num_eeg_channels (board_id);
            this.first_eeg_channel = BoardInfoGetter.get_first_eeg_channel (board_id);
        }

        public static void enable_board_logger ()
        {
            int res = Library.set_log_level (2);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public static void disable_board_logger ()
        {
            int res = Library.set_log_level (6);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public static void enable_dev_board_logger ()
        {
            int res = Library.set_log_level (0);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public void prepare_session ()
        {
            int res = Library.prepare_session (board_id, port_name);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public void config_board (string config)
        {
            int res = Library.config_board (config, board_id, port_name);
            if (res != (int)CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public void start_stream (int num_samples = 3600 * 250)
        {
            int res = Library.start_stream (num_samples, board_id, port_name);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public void stop_stream ()
        {
            int res = Library.stop_stream (board_id, port_name);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public void release_session ()
        {
            int res = Library.release_session (board_id, port_name);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public int get_board_data_count ()
        {
            int[] res = new int[1];
            int ec = Library.get_board_data_count (res, board_id, port_name);
            if (ec != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (ec);
            }
            return res[0];
        }

        public double[,] get_current_board_data (int num_samples)
        {
            float[] data_arr = new float[num_samples * package_length];
            double[] ts_arr = new double[num_samples];
            int[] current_size = new int[1];
            int ec = Library.get_current_board_data (num_samples, data_arr, ts_arr, current_size, board_id, port_name);
		    if (ec != (int) CustomExitCodes.STATUS_OK) {
			    throw new BrainFlowExceptioin (ec);
            }
            Array.Resize (ref data_arr, current_size[0] * package_length);
            Array.Resize (ref ts_arr, current_size[0]);
            return new BoardData (this.board_id, data_arr, ts_arr).get_board_data ();
	    }

        public double[,] get_immediate_board_data ()
        {
		        return get_current_board_data (0);
        }

        public double[,] get_board_data ()
        {
		    int size = get_board_data_count ();
		    float[] data_arr = new float[size * package_length];
		    double[] ts_arr = new double[size];
            int ec = Library.get_board_data (size, data_arr, ts_arr, board_id, port_name);
		    if (ec != (int) CustomExitCodes.STATUS_OK) {
                throw new BrainFlowExceptioin (ec);
            }
		    return new BoardData (this.board_id, data_arr, ts_arr).get_board_data ();
	    }
    }
}
