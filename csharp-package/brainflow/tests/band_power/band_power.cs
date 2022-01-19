using System;
using System.Runtime.Serialization;
using brainflow;
using brainflow.math;


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
            BoardDescr board_descr = BoardShim.get_board_descr<BoardDescr>(board_id);
            int sampling_rate = board_descr.sampling_rate;
            int nfft = DataFilter.get_nearest_power_of_two(sampling_rate);

            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            System.Threading.Thread.Sleep (10000);
            board_shim.stop_stream ();
            double[,] data = board_shim.get_board_data ();
            int[] eeg_channels = board_descr.eeg_channels;
            // use second channel of synthetic board to see 'alpha'
            int channel = eeg_channels[1];
            board_shim.release_session ();
            double[] detrend = DataFilter.detrend(data.GetRow(channel), (int)DetrendOperations.LINEAR);
            Tuple<double[], double[]> psd = DataFilter.get_psd_welch (detrend, nfft, nfft / 2, sampling_rate, (int)WindowFunctions.HANNING);
            double band_power_alpha = DataFilter.get_band_power (psd, 7.0, 13.0);
            double band_power_beta = DataFilter.get_band_power (psd, 14.0, 30.0);
            Console.WriteLine ("Alpha/Beta Ratio:" + (band_power_alpha/ band_power_beta));
        }
    }
}
