using System;
using System.Numerics;
using brainflow;

using Accord.Math;

namespace test
{
    class BandPower
    {
        static void Main (string[] args)
        {
            // use synthetic board for demo
            BoardShim.enable_dev_board_logger ();
            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            int board_id = (int)BoardIds.SYNTHETIC_BOARD;
            int sampling_rate = BoardShim.get_sampling_rate (board_id);

            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            System.Threading.Thread.Sleep (5000);
            board_shim.stop_stream ();
            double[,] data = board_shim.get_current_board_data (DataFilter.get_nearest_power_of_two (sampling_rate));
            int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
            board_shim.release_session ();

            for (int i = 0; i < eeg_channels.Length; i++)
            {
                // optional: you can subtract mean from signal before PSD calculation
                Tuple<double[], double[]> psd = DataFilter.get_psd (data.GetRow (eeg_channels[i]), 0,
                    data.GetRow (eeg_channels[i]).Length, sampling_rate, (int)WindowFunctions.HANNING);
                double band_power_alpha = DataFilter.get_band_power (psd, 7.0, 13.0);
                double band_power_beta = DataFilter.get_band_power (psd, 14.0, 30.0);
                Console.WriteLine ("Alpha/Beta Ratio:" + (band_power_alpha/ band_power_beta));
            }
        }
    }
}
