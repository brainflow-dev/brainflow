using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace brainflow
{
    public class BoardShim
    {
        public int board_id;
        public string port_name;
        public int fs_hz;
        public int num_eeg_channels;
        public int first_eeg_channel;
        public int last_eeg_channel;
        public int package_length;

        public BoardShim (int board_id, string port_name)
        {
            this.board_id = board_id;
            this.port_name = port_name;
            if (board_id == (int) BoardIds.CYTHON_BOARD)
            {
                this.package_length = Cython.package_length;
                this.fs_hz = Cython.fs_hz;
                this.num_eeg_channels = Cython.num_eeg_channels;
                this.first_eeg_channel = Cython.first_eeg_channel;
                this.last_eeg_channel = Cython.last_eeg_channel;
            }
            else
            {
                throw new BrainFlowExceptioin ((int)CustomExitCodes.UNSUPPORTED_BOARD_ERROR);
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

        public void start_stream (int num_samples = 3600 * 250)
        {
            int res = Library.start_stream(num_samples);
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public void stop_stream ()
        {
            int res = Library.stop_stream ();
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public void release_session ()
        {
            int res = Library.release_session ();
            if (res != (int) CustomExitCodes.STATUS_OK)
            {
                throw new BrainFlowExceptioin (res);
            }
        }

        public int get_board_data_count ()
        {
            int[] res = new int[1];
            int ec = Library.get_board_data_count (res);
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
            int ec = Library.get_current_board_data (num_samples, data_arr, ts_arr, current_size);
		    if (ec != (int) CustomExitCodes.STATUS_OK) {
			    throw new BrainFlowExceptioin (ec);
            }
            Array.Resize (ref data_arr, current_size[0] * package_length);
            Array.Resize (ref ts_arr, current_size[0]);
            return new BoardData ((int)BoardIds.CYTHON_BOARD, data_arr, ts_arr).get_board_data ();
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
            int ec = Library.get_board_data (size, data_arr, ts_arr);
		    if (ec != (int) CustomExitCodes.STATUS_OK) {
                throw new BrainFlowExceptioin (ec);
            }
		    return new BoardData ((int) BoardIds.CYTHON_BOARD, data_arr, ts_arr).get_board_data ();
	    }
    }
}
