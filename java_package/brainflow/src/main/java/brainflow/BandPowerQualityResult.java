package brainflow;

/** Band powers and quality reports for the same input window. */
public final class BandPowerQualityResult extends SignalQualityResult
{
    public final boolean valid;
    public final String reason;
    /** Normalized means, or null when invalid. */
    public final double[] avg_band_powers;
    /** Coefficients of variation, or null when invalid. */
    public final double[] stddev_band_powers;

    BandPowerQualityResult (int[] channels, int[] flags, double[] measurements, SignalQualityOptions options,
            int valid, double[] avgs, double[] stddevs)
    {
        super (channels, flags, measurements, options);
        this.valid = valid != 0;
        reason = this.valid ? "ok" :
                (used_channels.length < options.min_channels ? "insufficient_channels" : "no_band_power");
        avg_band_powers = this.valid ? avgs : null;
        stddev_band_powers = this.valid ? stddevs : null;
    }
}
