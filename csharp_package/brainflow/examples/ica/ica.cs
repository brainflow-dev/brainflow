using System;
using System.Runtime.Serialization;

using brainflow;
using brainflow.math;


namespace examples
{
    class ICA
    {
        static void Main (string[] args)
        {
            BoardShim.enable_dev_board_logger ();
            
            int board_id = (int)BoardIds.SYNTHETIC_BOARD;
            BoardDescr board_descr = BoardShim.get_board_descr<BoardDescr> (board_id);
            int[] eeg_channels = board_descr.eeg_channels;
            int channel = eeg_channels[1];

            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            System.Threading.Thread.Sleep (10000);
            board_shim.stop_stream ();
            double[,] data = board_shim.get_board_data (500);
            board_shim.release_session ();

            double[,] ica_data = data.GetRow (channel).Reshape(5, 100);
            Tuple<double[,], double[,], double[,], double[,]> ica = DataFilter.perform_ica (ica_data, 2);
        }
    }
}
