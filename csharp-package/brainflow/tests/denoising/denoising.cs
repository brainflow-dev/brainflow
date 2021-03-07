using System;

using brainflow;
using brainflow.math;


namespace test
{
    class Denoising
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
            double[,] unprocessed_data = board_shim.get_current_board_data (64);
            int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
            foreach (var index in eeg_channels)
                Console.WriteLine ("[{0}]", string.Join (", ", unprocessed_data.GetRow (index)));
            board_shim.release_session ();

            // for demo apply different methods to different channels
            double[] filtered;
            for (int i = 0; i < eeg_channels.Length; i++)
            {
                switch (i)
                {
                    // first of all you can try simple moving average or moving median
                    case 0:
                        filtered = DataFilter.perform_rolling_filter (unprocessed_data.GetRow (eeg_channels[i]), 3, (int)AggOperations.MEAN);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    case 1:
                        filtered = DataFilter.perform_rolling_filter (unprocessed_data.GetRow (eeg_channels[i]), 3, (int)AggOperations.MEDIAN);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                    // if for your signal these methods dont work good you can try wavelet based denoising
                    default:
                        // feel free to try different functions and different decomposition levels
                        filtered = DataFilter.perform_wavelet_denoising (unprocessed_data.GetRow (eeg_channels[i]), "db4", 3);
                        Console.WriteLine ("Filtered channel " + eeg_channels[i]);
                        Console.WriteLine ("[{0}]", string.Join (", ", filtered));
                        break;
                }
            }
        }
    }
}
