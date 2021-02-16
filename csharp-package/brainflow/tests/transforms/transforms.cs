using System;
using System.Numerics;

using brainflow;
using brainflow.math;


namespace test
{
    class Transforms
    {
        static void Main (string[] args)
        {
            // use synthetic board for demo
            BoardShim.enable_dev_board_logger ();
            BrainFlowInputParams input_params = new BrainFlowInputParams ();
            int board_id = (int)BoardIds.SYNTHETIC_BOARD;

            BoardShim board_shim = new BoardShim (board_id, input_params);
            board_shim.prepare_session ();
            board_shim.start_stream (3600);
            System.Threading.Thread.Sleep (5000);
            board_shim.stop_stream ();
            double[,] unprocessed_data = board_shim.get_current_board_data (64);
            int[] eeg_channels = BoardShim.get_eeg_channels (board_id);
            board_shim.release_session ();

            for (int i = 0; i < eeg_channels.Length; i++)
            {
                Console.WriteLine ("Original data:");
                Console.WriteLine ("[{0}]", string.Join (", ", unprocessed_data.GetRow (eeg_channels[i])));
                // demo for wavelet transform
                // tuple of coeffs array in format[A(J) D(J) D(J-1) ..... D(1)] where J is a
                // decomposition level, A - app coeffs, D - detailed coeffs, and array which stores
                // length for each block, len of this array is decomposition_length + 1
                Tuple<double[], int[]> wavelet_data = DataFilter.perform_wavelet_transform(unprocessed_data.GetRow (eeg_channels[i]), "db4", 3);
                // print app coeffs
                for (int j = 0; j < wavelet_data.Item2[0]; j++)
                {
                    Console.Write (wavelet_data.Item1[j] + " ");
                }
                Console.WriteLine ();
                // you can do smth with wavelet coeffs here, for example denoising works via thresholds for wavelets coeffs
                double[] restored_data = DataFilter.perform_inverse_wavelet_transform (wavelet_data, unprocessed_data.GetRow (eeg_channels[i]).Length, "db4", 3);
                Console.WriteLine ("Restored wavelet data:");
                Console.WriteLine ("[{0}]", string.Join (", ", restored_data));

                // demo for fft
                // end_pos - start_pos must be a power of 2
                Complex[] fft_data = DataFilter.perform_fft (unprocessed_data.GetRow (eeg_channels[i]), 0, 64, (int)WindowFunctions.HAMMING);
                // len of fft_data is N / 2 + 1
                double[] restored_fft_data = DataFilter.perform_ifft (fft_data);
                Console.WriteLine ("Restored fft data:");
                Console.WriteLine ("[{0}]", string.Join (", ", restored_fft_data));
            }
        }
    }
}
