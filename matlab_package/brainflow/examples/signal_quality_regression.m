function signal_quality_regression()
    samples = 1024;
    rate = 128;
    time = (0:samples-1) / rate;
    data = [sin(2*pi*10*time); zeros(1, samples); 2*sin(2*pi*20*time)];
    data(2, 1:3) = [NaN, 100, -100];
    channels = [3, 2, 1];
    options = struct('enable_clipping', true, 'lower_bound', -10, 'upper_bound', 10, ...
        'min_channels', 2);
    quality = DataFilter.get_signal_quality(data, channels, rate, options);
    report = quality.quality(2);
    assert(report.channel == 2 && report.flags == 7);
    assert(report.nonfinite_percentage == 100 / samples);
    assert(report.clipped_percentage == 200 / samples);
    assert(report.longest_flatline_seconds == (samples - 4) / rate);
    result = DataFilter.get_avg_band_powers_with_quality(data, channels, rate, false, options);
    [expected, variation] = DataFilter.get_avg_band_powers(data, [3, 1], rate, false);
    assert(result.valid && isequal(result.quality, quality.quality));
    assert(isequal(result.used_channels, [3, 1]) && isequal(result.rejected_channels, 2));
    assert(isequal(result.avg_band_powers, expected) && isequal(result.stddev_band_powers, variation));
    options.min_channels = 3;
    invalid = DataFilter.get_avg_band_powers_with_quality(data, channels, rate, false, options);
    assert(~invalid.valid && strcmp(invalid.reason, 'insufficient_channels'));
    assert(isempty(invalid.avg_band_powers) && isempty(invalid.stddev_band_powers));
end
