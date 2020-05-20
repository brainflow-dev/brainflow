package brainflow.examples;

import brainflow.AggOperations;
import brainflow.BoardIds;
import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;
import brainflow.DataFilter;
import brainflow.LogLevels;

import java.util.Arrays;

public class Denoising
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
        int num_rows = BoardShim.get_num_rows (board_id);
        double[][] data = board_shim.get_current_board_data (64);
        for (int i = 0; i < num_rows; i++)
        {
            System.out.println (Arrays.toString (data[i]));
        }
        board_shim.release_session ();

        int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
        for (int i = 0; i < eeg_channels.length; i++)
        {
            // just for demo - apply different methods to different eeg channels
            switch (i)
            {
                // first of all you can try simple moving average or moving median
                case 0:
                    DataFilter.perform_rolling_filter (data[eeg_channels[i]], 3, AggOperations.MEAN.get_code ());
                    break;
                case 1:
                    DataFilter.perform_rolling_filter (data[eeg_channels[i]], 3, AggOperations.MEDIAN.get_code ());
                    break;
                // if methods above dont work good for you you should try wavelet based
                // denoising
                default:
                    // try different functions and different decomposition levels here
                    DataFilter.perform_wavelet_denoising (data[eeg_channels[i]], "db4", 3);
                    break;
            }
        }
        System.out.println ("After signal processing:");
        for (int i = 0; i < num_rows; i++)
        {
            System.out.println (Arrays.toString (data[i]));
        }
    }
}
