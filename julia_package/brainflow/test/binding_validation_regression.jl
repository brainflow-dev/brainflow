using Test
using BrainFlow

@testset "Binding validation" begin
    @test_throws BrainFlow.BrainFlowError BrainFlow.get_band_power((ones(2), ones(1)), 0.0, 1.0)
    @test_throws BrainFlow.BrainFlowError BrainFlow.get_csp(ones(4, 2, 20), zeros(3))
    @test_throws BrainFlow.BrainFlowError BrainFlow.perform_inverse_wavelet_transform(
        (zeros(1), Cint[19, 19, 34]), 64, BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
    data = sin.(2pi .* 10 .* (0:1024) ./ 256)
    amplitudes, frequencies = BrainFlow.get_psd_welch(data, 256, 128, 256, BrainFlow.HANNING)
    @test length(amplitudes) == 129
    @test length(frequencies) == 129
    @test frequencies[end] == 128
    transformed = BrainFlow.perform_wavelet_transform(data, BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
    restored = BrainFlow.perform_inverse_wavelet_transform(transformed, length(data), BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
    @test restored ≈ data atol=1e-10
    @test_throws BrainFlow.BrainFlowError BrainFlow.perform_inverse_wavelet_transform(
        transformed, 64, BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
    @test_throws BrainFlow.BrainFlowError BrainFlow.perform_inverse_wavelet_transform(
        transformed, typemax(Cint), BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
    for values in (view(data, length(data):-1:1), view(data, 1:2:length(data)))
        @test BrainFlow.calc_stddev(values) ≈ BrainFlow.calc_stddev(collect(values))
        @test_throws BrainFlow.BrainFlowError BrainFlow.detrend(values, BrainFlow.CONSTANT)
        actual = BrainFlow.perform_wavelet_transform(values, BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
        expected = BrainFlow.perform_wavelet_transform(collect(values), BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
        @test actual == expected
    end
    contiguous = copy(data)
    view_to_process = view(contiguous, 2:20)
    expected = copy(view_to_process)
    BrainFlow.detrend(view_to_process, BrainFlow.CONSTANT)
    BrainFlow.detrend(expected, BrainFlow.CONSTANT)
    @test view_to_process == expected
end

@testset "Signal quality" begin
    samples, rate = 1024, 128
    time = (0:samples-1) ./ rate
    data = [sin.(2pi .* 10 .* time)'; zeros(1, samples); (2 .* sin.(2pi .* 20 .* time))']
    data[2, 1:3] = [NaN, 100, -100]
    channels = [3, 2, 1]
    options = BrainFlow.SignalQualityOptions(clipping_bounds=(-10.0, 10.0), min_channels=2)
    quality = BrainFlow.get_signal_quality(data, channels, rate, options)
    report = quality.quality[2]
    @test report.channel == 2
    @test report.flags == (BrainFlow.QUALITY_NONFINITE | BrainFlow.QUALITY_CLIPPED | BrainFlow.QUALITY_FLATLINE)
    @test report.nonfinite_percentage == 100 / samples
    @test report.clipped_percentage == 200 / samples
    @test report.longest_flatline_seconds == (samples - 4) / rate
    result = BrainFlow.get_avg_band_powers_with_quality(data, channels, rate, false, options)
    expected = BrainFlow.get_avg_band_powers(data, [3, 1], rate, false)
    @test result.valid
    @test result.quality == quality.quality
    @test result.used_channels == [3, 1]
    @test result.rejected_channels == [2]
    @test (result.avg_band_powers, result.stddev_band_powers) == expected
    options = BrainFlow.SignalQualityOptions(clipping_bounds=(-10.0, 10.0), min_channels=3)
    invalid = BrainFlow.get_avg_band_powers_with_quality(data, channels, rate, false, options)
    @test !invalid.valid
    @test invalid.reason == :insufficient_channels
    @test invalid.avg_band_powers === nothing
    @test invalid.stddev_band_powers === nothing
    @test_throws BrainFlow.BrainFlowError BrainFlow.get_signal_quality(data, [1, 1], rate)
end
