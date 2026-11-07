package brainflow;

/** Thresholds for opt-in, whole-channel quality checks. */
public final class SignalQualityOptions
{
    public final boolean enable_clipping;
    public final double lower_bound;
    public final double upper_bound;
    public final double max_clipped_percentage;
    public final double flatline_tolerance;
    public final double max_flatline_seconds;
    public final int min_channels;

    public SignalQualityOptions ()
    {
        this (false, 0, 0, 0, 0, 0.5, 1);
    }

    public SignalQualityOptions (boolean enable_clipping, double lower_bound, double upper_bound,
            double max_clipped_percentage, double flatline_tolerance, double max_flatline_seconds, int min_channels)
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
