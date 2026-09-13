using System;

using brainflow;
using brainflow.math;


namespace examples
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
            double[,] unprocessed_data = board_shim.get_board_data ();
            int[] eeg_channels = BoardShim.get_eeg_channels (board_id);

            // Re-reference every EEG channel using the sample-wise mean of the first two EEG channels.
            // The operation changes unprocessed_data in-place.
            if (eeg_channels.Length >= 2)
            {
                int[] reference_channels = {eeg_channels[0], eeg_channels[1]};
                DataFilter.reference (unprocessed_data, eeg_channels, reference_channels);
            }

            board_shim.release_session ();

            for (int i = 0; i < eeg_channels.Length; i++)
            {
                Console.WriteLine ("Before processing:");
                Console.WriteLine ("[{0}]", string.Join (", ", unprocessed_data.GetRow (eeg_channels[i])));
                // you can use MEAN, MEDIAN or EACH for downsampling
                double[] filtered = DataFilter.perform_downsampling (unprocessed_data.GetRow (eeg_channels[i]), 3, (int)AggOperations.MEDIAN);
                Console.WriteLine ("Before processing:");
                Console.WriteLine ("[{0}]", string.Join (", ", filtered));
            }
        }
    }
}
