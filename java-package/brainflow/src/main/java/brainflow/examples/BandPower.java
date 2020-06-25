package brainflow.examples;

import java.util.Arrays;

import org.apache.commons.lang3.tuple.Pair;
import org.apache.commons.math3.complex.Complex;

import brainflow.BoardIds;
import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;
import brainflow.DataFilter;
import brainflow.LogLevels;
import brainflow.WindowFunctions;

public class BandPower
{

    public static void main (String[] args) throws Exception
    {
        // use synthetic board for demo
        BoardShim.enable_board_logger ();
        BrainFlowInputParams params = new BrainFlowInputParams ();
        int board_id = BoardIds.SYNTHETIC_BOARD.get_code ();
        BoardShim board_shim = new BoardShim (board_id, params);
        int sampling_rate = BoardShim.get_sampling_rate (board_id);

        board_shim.prepare_session ();
        board_shim.start_stream (3600);
        BoardShim.log_message (LogLevels.LEVEL_INFO.get_code (), "Start sleeping in the main thread");
        Thread.sleep (10000);
        board_shim.stop_stream ();
        int num_rows = BoardShim.get_num_rows (board_id);
        double[][] data = board_shim.get_current_board_data (DataFilter.get_nearest_power_of_two (sampling_rate));
        board_shim.release_session ();

        int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
        for (int i = 0; i < eeg_channels.length; i++)
        {
            // optional: you can subtract mean before calculation
            // here we know that len is power of 2
            Pair<double[], double[]> psd = DataFilter.get_psd (data[eeg_channels[i]], 0, data[eeg_channels[i]].length,
                    sampling_rate, WindowFunctions.HAMMING.get_code ());
            double band_power_alpha = DataFilter.get_band_power (psd, 7.0, 13.0);
            double band_power_beta = DataFilter.get_band_power (psd, 14.0, 30.0);
            System.out.println ("Alpha/Beta Ratio: " + (band_power_alpha / band_power_beta));
        }
    }
}
