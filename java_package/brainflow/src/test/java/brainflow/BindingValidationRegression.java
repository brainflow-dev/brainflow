package brainflow;

import org.apache.commons.lang3.tuple.Pair;
import java.util.Arrays;
import java.util.List;

/** Run with the current native DataHandler library on the classpath/library path. */
public class BindingValidationRegression
{
    interface CheckedCall
    {
        void run () throws Exception;
    }

    private static void rejects (CheckedCall call) throws Exception
    {
        try
        {
            call.run ();
        } catch (BrainFlowError expected)
        {
            return;
        }
        throw new AssertionError ("Expected BrainFlowError");
    }

    public static void main (String[] args) throws Exception
    {
        rejects (() -> DataFilter.calc_stddev (new double[2], -1, 2));
        rejects (() -> DataFilter.calc_stddev (new double[2], 0, 3));
        rejects (() -> DataFilter.get_band_power (Pair.of (new double[2], new double[1]), 0, 1));
        rejects (() -> DataFilter.get_csp (new double[4][2][20], new double[3]));
        rejects (() -> DataFilter.perform_inverse_wavelet_transform (
                Pair.of (new double[1], new int[] {19, 19, 34}), 64, WaveletTypes.DB3, 2,
                WaveletExtensionTypes.SYMMETRIC));
        double[] signal = new double[64];
        for (int i = 0; i < signal.length; i++)
        {
            signal[i] = Math.sin (i * 0.3);
        }
        Pair<double[], int[]> transformed = DataFilter.perform_wavelet_transform (
                signal, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC);
        double[] restored = DataFilter.perform_inverse_wavelet_transform (
                transformed, signal.length, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC);
        for (int i = 0; i < signal.length; i++)
        {
            if (Math.abs (signal[i] - restored[i]) > 1e-10)
            {
                throw new AssertionError ("Wavelet roundtrip failed");
            }
        }
        rejects (() -> DataFilter.perform_inverse_wavelet_transform (
                transformed, 32, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC));
        rejects (() -> DataFilter.perform_inverse_wavelet_transform (
                transformed, Integer.MAX_VALUE, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC));
        rejects (() -> DataFilter.perform_wavelet_transform (
                signal, WaveletTypes.DB3, Integer.MAX_VALUE, WaveletExtensionTypes.SYMMETRIC));
        rejects (() -> DataFilter.perform_wavelet_transform (
                signal, WaveletTypes.DB3, 101, WaveletExtensionTypes.SYMMETRIC));
        rejects (() -> DataFilter.perform_ica (new double[][] {new double[64], new double[32]},
                2, new int[] {0, 1}));
        rejects (() -> DataFilter.get_custom_band_powers (
                new double[][] {new double[64], new double[32]},
                Arrays.asList (Pair.of (2.0, 4.0)), new int[] {0, 1}, 128, false));
        rejects (() -> DataFilter.perform_ica (new double[2][64], 2, new int[] {-1, 0}));

        // Unselected rows do not determine native input/output dimensions. Previously,
        // S was allocated using row zero while native ICA wrote the selected row length.
        double[][] mixed = new double[][] {new double[1], new double[1024], new double[1024]};
        for (int i = 0; i < 1024; i++)
        {
            double time = i / 256.0;
            double first = Math.sin (2.0 * Math.PI * 7.0 * time);
            double second = Math.pow (Math.sin (2.0 * Math.PI * 13.0 * time), 3);
            mixed[1][i] = first + 0.3 * second;
            mixed[2][i] = 0.2 * first + second;
        }
        List<double[][]> decomposition = DataFilter.perform_ica (mixed, 2, new int[] {2, 1});
        double[][] mixing = decomposition.get (2);
        double[][] sources = decomposition.get (3);
        if (sources.length != 2 || sources[0].length != 1024)
        {
            throw new AssertionError ("ICA source dimensions must follow selected channels");
        }
        for (int row = 0; row < 2; row++)
        {
            for (int i = 0; i < 1024; i++)
            {
                double restored_sample = mixing[row][0] * sources[0][i] + mixing[row][1] * sources[1][i];
                if (Math.abs (restored_sample - mixed[2 - row][i]) > 1e-9)
                {
                    throw new AssertionError ("ICA channel-order reconstruction failed");
                }
            }
        }
        System.out.println ("Binding validation regressions passed");
    }
}
