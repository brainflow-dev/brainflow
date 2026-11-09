using System;
using System.Collections.Generic;

namespace brainflow
{
    [Flags]
    public enum SignalQualityFlags
    {
        GOOD = 0,
        NONFINITE = 1,
        CLIPPED = 2,
        FLATLINE = 4
    }

    /// <summary>Thresholds for opt-in, whole-channel quality checks.</summary>
    public sealed class SignalQualityOptions
    {
        public bool enable_clipping { get; }
        public double lower_bound { get; }
        public double upper_bound { get; }
        public double max_clipped_percentage { get; }
        public double flatline_tolerance { get; }
        public double max_flatline_seconds { get; }
        public int min_channels { get; }

        public SignalQualityOptions (bool enable_clipping = false, double lower_bound = 0,
            double upper_bound = 0, double max_clipped_percentage = 0, double flatline_tolerance = 0,
            double max_flatline_seconds = 0.5, int min_channels = 1)
        {
            this.enable_clipping = enable_clipping;
            this.lower_bound = lower_bound;
            this.upper_bound = upper_bound;
            this.max_clipped_percentage = max_clipped_percentage;
            this.flatline_tolerance = flatline_tolerance;
            this.max_flatline_seconds = max_flatline_seconds;
            this.min_channels = min_channels;
        }
    }

    public sealed class ChannelQuality
    {
        public int channel { get; }
        public SignalQualityFlags flags { get; }
        public double nonfinite_percentage { get; }
        public double clipped_percentage { get; }
        public double flatline_percentage { get; }
        public double longest_flatline_seconds { get; }

        internal ChannelQuality (int channel, int flags, double[] measurements, int offset)
        {
            this.channel = channel;
            this.flags = (SignalQualityFlags)flags;
            nonfinite_percentage = measurements[offset];
            clipped_percentage = measurements[offset + 1];
            flatline_percentage = measurements[offset + 2];
            longest_flatline_seconds = measurements[offset + 3];
        }
    }

    public class SignalQualityResult
    {
        public int[] channels { get; }
        public ChannelQuality[] quality { get; }
        public int[] used_channels { get; }
        public int[] rejected_channels { get; }
        public SignalQualityOptions quality_policy { get; }

        internal SignalQualityResult (int[] channels, int[] flags, double[] measurements,
            SignalQualityOptions options)
        {
            this.channels = (int[])channels.Clone ();
            quality_policy = options;
            quality = new ChannelQuality[channels.Length];
            var used = new List<int> ();
            var rejected = new List<int> ();
            for (int row = 0; row < channels.Length; row++)
            {
                quality[row] = new ChannelQuality (channels[row], flags[row], measurements, 4 * row);
                (flags[row] == 0 ? used : rejected).Add (channels[row]);
            }
            used_channels = used.ToArray ();
            rejected_channels = rejected.ToArray ();
        }
    }

    public sealed class BandPowerQualityResult : SignalQualityResult
    {
        public bool valid { get; }
        public string reason { get; }
        /// <summary>Normalized means. Null when invalid.</summary>
        public double[] avg_band_powers { get; }
        /// <summary>Coefficients of variation. Null when invalid.</summary>
        public double[] stddev_band_powers { get; }

        internal BandPowerQualityResult (int[] channels, int[] flags, double[] measurements,
            SignalQualityOptions options, int valid, double[] avgs, double[] stddevs)
            : base (channels, flags, measurements, options)
        {
            this.valid = valid != 0;
            reason = this.valid ? "ok" :
                (used_channels.Length < options.min_channels ? "insufficient_channels" : "no_band_power");
            avg_band_powers = this.valid ? avgs : null;
            stddev_band_powers = this.valid ? stddevs : null;
        }
    }
}
