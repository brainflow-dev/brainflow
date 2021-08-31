package brainflow.examples;

import java.util.Arrays;

import brainflow.AggOperations;
import brainflow.BoardIds;
import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;
import brainflow.DataFilter;
import brainflow.LogLevels;

public class Downsampling
{

    public static void main (String[] args) throws Exception
    {
        // use synthetic board for demo
        BoardShim.enable_board_logger ();
        BrainFlowInputParams params = new BrainFlowInputParams ();
        int board_id = BoardIds.SYNTHETIC_BOARD.get_code ();

        BoardShim board_shim = new BoardShim (board_id, params);
        board_shim.prepare_session ();
        board_shim.start_stream (3600);
        BoardShim.log_message (LogLevels.LEVEL_INFO.get_code (), "Start sleeping in the main thread");
        Thread.sleep (5000);
        board_shim.stop_stream ();
        System.out.println (board_shim.get_board_data_count ());
        double[][] data = board_shim.get_board_data (30);
        board_shim.release_session ();

        int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
        for (int i = 0; i < eeg_channels.length; i++)
        {
            System.out.println ("Original data:");
            System.out.println (Arrays.toString (data[i]));
            // keep each second element, you can use MEAN and MEDIAN as well
            double[] downsampled_data = DataFilter.perform_downsampling (data[eeg_channels[i]], 2,
                    AggOperations.EACH.get_code ());
            System.out.println ("Downsampled data:");
            System.out.println (Arrays.toString (downsampled_data));
        }
    }
}
