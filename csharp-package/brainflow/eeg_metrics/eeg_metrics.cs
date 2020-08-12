using System;
using System.Numerics;
using brainflow;

using Accord.Math;

namespace test
{
    class EEGMetrics
    {
        static void Main (string[] args)
        {
            // use synthetic board for demo
            BoardShim.enable_dev_board_logger ();
            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            int board_id = (int)BoardIds.SYNTHETIC_BOARD;
            int sampling_rate = BoardShim.get_sampling_rate (board_id);
            int nfft = DataFilter.get_nearest_power_of_two(sampling_rate);
            int[] eeg_channels = BoardShim.get_eeg_channels(board_id);

            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            System.Threading.Thread.Sleep (10000);
            board_shim.stop_stream ();
            double[,] data = board_shim.get_board_data ();
            board_shim.release_session ();

            Tuple<double[], double[]> bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, true);
            double[] feature_vector = bands.Item1.Concatenate (bands.Item2);
            MLModel concentration = new MLModel((int)BrainFlowMetrics.CONCENTRATION, (int)BrainFlowClassifiers.REGRESSION);
            concentration.prepare ();
            Console.WriteLine ("Concentration: " + concentration.predict (feature_vector));
            concentration.release ();
        }
    }
}
