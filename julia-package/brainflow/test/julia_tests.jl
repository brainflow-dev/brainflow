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