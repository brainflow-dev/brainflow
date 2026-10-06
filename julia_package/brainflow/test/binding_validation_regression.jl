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
