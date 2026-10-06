using BrainFlow
using Test

@testset "ML prediction buffers and input layout" begin
    features = Float64[.1, .1, .3, .3, .2]
    params = BrainFlowModelParams(BrainFlow.MINDFULNESS, BrainFlow.DEFAULT_CLASSIFIER)
    params.max_array_size = 2
    BrainFlow.prepare(params)
    try
        expected = BrainFlow.predict(features, params)
        @test length(expected) == 1
        @test isfinite(expected[1]) && 0 <= expected[1] <= 1
        backing = repeat(features, inner=2)
        @test BrainFlow.predict(@view(backing[1:2:end]), params) == expected
        @test BrainFlow.predict(@view(reverse(features)[end:-1:1]), params) == expected
        @test_throws BrainFlow.BrainFlowError BrainFlow.predict(reshape(features, 1, 5), params)
        @test_throws BrainFlow.BrainFlowError BrainFlow.predict(Float64[], params)
        @test_throws BrainFlow.BrainFlowError BrainFlow.predict([NaN, .1, .3, .3, .2], params)
        @test_throws BrainFlow.BrainFlowError BrainFlow.predict(ComplexF64.(features), params)
    finally
        BrainFlow.release(params)
    end
    params.max_array_size = 0
    @test_throws BrainFlow.BrainFlowError BrainFlow.prepare(params)
    @test_throws BrainFlow.BrainFlowError BrainFlow.predict(features, params)
end

if haskey(ENV, "BRAINFLOW_TEST_ONNX_MODEL")
    @testset "Two-output ONNX prediction" begin
        params = BrainFlowModelParams(BrainFlow.USER_DEFINED, BrainFlow.ONNX_CLASSIFIER)
        params.file = ENV["BRAINFLOW_TEST_ONNX_MODEL"]
        params.output_name = get(ENV, "BRAINFLOW_TEST_ONNX_OUTPUT", "probabilities")
        params.max_array_size = 2
        BrainFlow.prepare(params)
        try
            for _ in 1:20
                scores = BrainFlow.predict(Float64[.1, .1, .3, .3, .2], params)
                @test length(scores) == 2
                @test all(isfinite, scores)
                @test sum(scores) ≈ 1.0 atol=1e-6
            end
            # A changed parameter object describes a different native model,
            # and must fail lookup rather than write to a smaller buffer.
            params.max_array_size = 1
            @test_throws BrainFlow.BrainFlowError BrainFlow.predict(ones(5), params)
            params.max_array_size = 2
        finally
            params.max_array_size = 2
            BrainFlow.release(params)
        end
    end
end
