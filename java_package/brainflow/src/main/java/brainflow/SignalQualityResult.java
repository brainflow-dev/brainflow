package brainflow;

import java.util.Arrays;

/** Reports for the selected channels, in selection order. */
public class SignalQualityResult
{
    public static final int GOOD = 0;
    public static final int NONFINITE = 1;
    public static final int CLIPPED = 2;
    public static final int FLATLINE = 4;

    public static final class ChannelQuality
    {
        public final int channel;
        /** Bit mask of the quality flags. */
        public final int flags;
        public final double nonfinite_percentage;
        public final double clipped_percentage;
        public final double flatline_percentage;
        public final double longest_flatline_seconds;

        ChannelQuality (int channel, int flags, double[] measurements, int offset)
        {
            this.channel = channel;
            this.flags = flags;
            nonfinite_percentage = measurements[offset];
            clipped_percentage = measurements[offset + 1];
            flatline_percentage = measurements[offset + 2];
            longest_flatline_seconds = measurements[offset + 3];
        }
    }

    public final int[] channels;
    public final ChannelQuality[] quality;
    public final int[] used_channels;
    public final int[] rejected_channels;
    public final SignalQualityOptions quality_policy;

    SignalQualityResult (int[] channels, int[] flags, double[] measurements, SignalQualityOptions options)
    {
        this.channels = channels.clone ();
        quality_policy = options;
        quality = new ChannelQuality[channels.length];
        int[] used = new int[channels.length];
        int[] rejected = new int[channels.length];
        int used_count = 0;
        int rejected_count = 0;
        for (int row = 0; row < channels.length; row++)
        {
            quality[row] = new ChannelQuality (channels[row], flags[row], measurements, 4 * row);
            if (flags[row] == GOOD)
            {
                used[used_count++] = channels[row];
            }
            else
            {
                rejected[rejected_count++] = channels[row];
            }
        }
        used_channels = Arrays.copyOf (used, used_count);
        rejected_channels = Arrays.copyOf (rejected, rejected_count);
    }
}
