package brainflow.examples;

import java.util.List;

import brainflow.BoardIds;
import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;
import brainflow.DataFilter;

public class ICA
{

    public static void main (String[] args) throws Exception
    {
        // use synthetic board for demo
        BoardShim.enable_board_logger ();
        BrainFlowInputParams params = new BrainFlowInputParams ();
        BoardIds board_id = BoardIds.SYNTHETIC_BOARD;
        BoardShim board_shim = new BoardShim (board_id, params);
        board_shim.prepare_session ();
        board_shim.start_stream (3600);
        Thread.sleep (10000);
        board_shim.stop_stream ();
        double[][] data = board_shim.get_board_data (500);
        board_shim.release_session ();

        int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
        int eeg_channel = eeg_channels[1];
        double[][] ica_data = DataFilter.reshape_data_to_2d (5, 100, data[eeg_channel]);
        List<double[][]> ica = DataFilter.perform_ica (ica_data, 2);
        System.out.println ("Completed");
    }
}
