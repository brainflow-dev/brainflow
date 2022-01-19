using System;

using brainflow;
using brainflow.math;


namespace test
{
    class Downsampling
    {
        static void Main (string[] args)
        {
            // use synthetic board for demo
            BoardShim.enable_dev_board_logger ();
            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            int board_id = (int)BoardIds.SYNTHETIC_BOARD;

            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            System.Threading.Thread.Sleep (5000);
            board_shim.stop_stream ();
            double[,] unprocessed_data = board_shim.get_board_data (20);
            int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
            board_shim.release_session ();

            for (int i = 0; i < eeg_channels.Length; i++)
            {
                Console.WriteLine ("Before processing:");
                Console.WriteLine ("[{0}]", string.Join (", ", unprocessed_data.GetRow(eeg_channels[i])));
                // you can use MEAN, MEDIAN or EACH for downsampling
                double[] filtered = DataFilter.perform_downsampling (unprocessed_data.GetRow (eeg_channels[i]), 3, (int)AggOperations.MEDIAN);
                Console.WriteLine ("Before processing:");
                Console.WriteLine ("[{0}]", string.Join (", ", filtered));
            }
        }
    }
}
