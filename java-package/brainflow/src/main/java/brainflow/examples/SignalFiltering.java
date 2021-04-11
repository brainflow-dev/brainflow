package brainflow.examples;

import java.util.Arrays;

import brainflow.BoardIds;
import brainflow.BoardShim;
import brainflow.BrainFlowInputParams;
import brainflow.DataFilter;
import brainflow.FilterTypes;
import brainflow.LogLevels;
import brainflow.NoiseTypes;

public class SignalFiltering
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
        double[][] data = board_shim.get_current_board_data (30);
        for (int i = 0; i < num_rows; i++)
        {
            System.out.println (Arrays.toString (data[i]));
        }
        board_shim.release_session ();

        int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
        for (int i = 0; i < eeg_channels.length; i++)
        {
            // just for demo - apply different filters to different eeg channels
            switch (i)
            {
                case 0:
                    DataFilter.perform_lowpass (data[eeg_channels[i]], BoardShim.get_sampling_rate (board_id), 20.0, 4,
                            FilterTypes.BESSEL.get_code (), 0.0);
                    break;
                case 1:
                    DataFilter.perform_highpass (data[eeg_channels[i]], BoardShim.get_sampling_rate (board_id), 5.0, 4,
                            FilterTypes.BUTTERWORTH.get_code (), 0.0);
                    break;
                case 2:
                    DataFilter.perform_bandpass (data[eeg_channels[i]], BoardShim.get_sampling_rate (board_id), 15.0,
                            5.0, 4, FilterTypes.CHEBYSHEV_TYPE_1.get_code (), 1.0);
                    break;
                case 3:
                    DataFilter.perform_bandstop (data[eeg_channels[i]], BoardShim.get_sampling_rate (board_id), 50.0,
                            1.0, 4, FilterTypes.CHEBYSHEV_TYPE_1.get_code (), 1.0);
                    break;
                default:
                    DataFilter.remove_environmental_noise (data[eeg_channels[i]],
                            BoardShim.get_sampling_rate (board_id), NoiseTypes.FIFTY.get_code ());;
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
