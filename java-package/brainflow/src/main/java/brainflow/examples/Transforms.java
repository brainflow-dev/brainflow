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

public class Transforms
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
        Thread.sleep (10000);
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
            System.out.println ("Original data:");
            System.out.println (Arrays.toString (data[eeg_channels[i]]));
            // demo for wavelet transform
            // Pair of coeffs array in format[A(J) D(J) D(J-1) ..... D(1)] where J is a
            // decomposition level, A - app coeffs, D - detailed coeffs, and array which
            // stores
            // length for each block, len of this array is decomposition_length + 1
            Pair<double[], int[]> wavelet_data = DataFilter.perform_wavelet_transform (data[eeg_channels[i]], "db4", 3);
            // print approximation coeffs
            for (int j = 0; j < wavelet_data.getRight ()[0]; j++)
            {
                System.out.print (wavelet_data.getLeft ()[j] + " ");
            }
            System.out.println ();
            // you can do smth with these coeffs here, for example denoising works via
            // thresholds for wavelet coeffs
            double[] restored_data = DataFilter.perform_inverse_wavelet_transform (wavelet_data,
                    data[eeg_channels[i]].length, "db4", 3);
            System.out.println ("Restored data after wavelet:");
            System.out.println (Arrays.toString (restored_data));

            // demo for fft works only for power of 2
            // len of fft_data is N / 2 + 1
            Complex[] fft_data = DataFilter.perform_fft (data[eeg_channels[i]], 0, 64,
                    WindowFunctions.NO_WINDOW.get_code ());
            double[] restored_fft_data = DataFilter.perform_ifft (fft_data);
            System.out.println ("Restored data after fft:");
            System.out.println (Arrays.toString (restored_fft_data));
        }
    }
}
