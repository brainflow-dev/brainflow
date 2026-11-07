using System;

namespace brainflow
{
    /// <summary>
    /// Reusable band-power workspace. Bounds limit native scratch storage; workers=0 chooses up to
    /// four persistent workers and workers=1 is serial. Dispose promptly to release workers.
    /// Calls on one context serialize. Uses the existing windowed, zero-phase preprocessing.
    /// </summary>
    public sealed class BandPowerContext : IDisposable
    {
        private int handle;
        private readonly int band_count;

        public BandPowerContext (Tuple<double, double>[] bands, int sampling_rate,
            int max_channels, int max_samples, bool apply_filters = true, int nfft = 0,
            double low_cutoff = 0, double high_cutoff = 0, int mains = -1,
            int? detrend_operation = null, int workers = 0)
        {
            if (bands == null || bands.Length == 0)
                throw new BrainFlowError ((int)BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR);
            band_count = bands.Length;
            double[] starts = new double[band_count], stops = new double[band_count];
            for (int i = 0; i < band_count; i++)
            {
                if (bands[i] == null)
                    throw new BrainFlowError ((int)BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR);
                starts[i] = bands[i].Item1;
                stops[i] = bands[i].Item2;
            }
            int[] created = new int[1];
            Check (DataHandlerLibrary.create_band_power_context (max_channels, max_samples,
                starts, stops, band_count, sampling_rate, apply_filters ? 1 : 0, nfft,
                low_cutoff, high_cutoff, mains, detrend_operation ?? (apply_filters ? 1 : 0),
                workers, created));
            handle = created[0];
        }

        private static void Check (int result)
        {
            if (result != (int)BrainFlowExitCodes.STATUS_OK)
                throw new BrainFlowError (result);
        }

        /// <summary>Process selected rows directly into reusable output arrays.</summary>
        public void process (double[,] data, int[] channels, double[] averages, double[] deviations)
        {
            if (handle == 0 || data == null || channels == null || averages == null ||
                deviations == null || averages.Length != band_count || deviations.Length != band_count ||
                ReferenceEquals (averages, deviations))
                throw new BrainFlowError ((int)BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR);
            Check (DataHandlerLibrary.process_band_power_context (handle, data, data.Length,
                data.GetLength (0), data.GetLength (1), data.GetLength (1), channels,
                channels.Length, averages, deviations, band_count));
        }

        public Tuple<double[], double[]> process (double[,] data, int[] channels)
        {
            double[] averages = new double[band_count], deviations = new double[band_count];
            process (data, channels, averages, deviations);
            return Tuple.Create (averages, deviations);
        }

        public void Dispose ()
        {
            int released = System.Threading.Interlocked.Exchange (ref handle, 0);
            if (released != 0)
                Check (DataHandlerLibrary.release_band_power_context (released));
            GC.SuppressFinalize (this);
        }

        ~BandPowerContext ()
        {
            try { Dispose (); }
            catch { /* The native library may be unavailable during shutdown. */ }
        }
    }
}
