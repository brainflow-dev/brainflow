using Test
using brainflow
using JSON

@testset "prepare_session error" begin
    # use a board that is not connected
    params = brainflow.BrainFlowInputParams()
    board_shim = brainflow.BoardShim(brainflow.CYTON_BOARD, params)
    @test_throws brainflow.BrainFlowError("Error in prepare_session 13", 13) brainflow.prepare_session(board_shim)
end

@testset "generated functions" begin
    @test brainflow.get_eeg_channels isa Function
end

@testset "data filtering" begin
    @test Int32(brainflow.CONSTANT) == 1
    @test Int32(brainflow.LINEAR) == 2

    @test Int32(brainflow.MEAN) == 0
    @test Int32(brainflow.MEDIAN) == 1
    @test Int32(brainflow.EACH) == 2

    @test Int32(brainflow.BUTTERWORTH) == 0
    @test Int32(brainflow.CHEBYSHEV_TYPE_1) == 1
    @test Int32(brainflow.BESSEL) == 2

    @test Int32(brainflow.NO_WINDOW) == 0
    @test Int32(brainflow.HANNING) == 1
    @test Int32(brainflow.HAMMING) == 2
    @test Int32(brainflow.BLACKMAN_HARRIS) == 3
end

@testset "model params" begin
    params = BrainFlowModelParams(metric = "concentration")
    @test params.metric == brainflow.Concentration()

    params = BrainFlowModelParams(metric = "Relaxation", classifier = "KNN")
    @test params.metric == brainflow.Relaxation()
    @test params.classifier == brainflow.Knn()

    json = JSON.json(BrainFlowModelParams())
    d = JSON.parse(json)
    @test d["classifier"] == 0
    @test d["metric"] == 0
    @test d["file"] == ""
    @test d["other_info"] == ""
end