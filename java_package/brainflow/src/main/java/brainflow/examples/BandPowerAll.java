package brainflow.examples;

import org.apache.commons.lang3.tuple.Pair;

import brainflow.BoardIds;
import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;
import brainflow.DataFilter;
import brainflow.LogLevels;

public class BandPowerAll
{

    public static void main (String[] args) throws Exception
    {
        BoardShim.enable_board_logger ();
        BrainFlowInputParams params = new BrainFlowInputParams ();
        BoardIds board_id = BoardIds.SYNTHETIC_BOARD;
        BoardShim board_shim = new BoardShim (board_id, params);
        int sampling_rate = BoardShim.get_sampling_rate (board_id);
        int[] eeg_channels = BoardShim.get_eeg_channels (board_id);

        board_shim.prepare_session ();
        board_shim.start_stream (3600);
        BoardShim.log_message (LogLevels.LEVEL_INFO, "Start sleeping in the main thread");
        Thread.sleep (10000);
        board_shim.stop_stream ();
        double[][] data = board_shim.get_board_data ();
        board_shim.release_session ();

        Pair<double[], double[]> bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, true);
        for (int i = 0; i < 5; i++)
        {
            System.out.println ("Avg: " + bands.getLeft ()[i] + " Stddev: " + bands.getRight ()[i]);
        }
    }
}
