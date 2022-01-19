using Test
using BrainFlow
using JSON

@testset "BoardShim constructor" begin
    # for most boards the master_id == the regular id
    params = BrainFlow.BrainFlowInputParams()
    board_shim = BrainFlow.BoardShim(BrainFlow.CYTON_BOARD, params)
    @test board_shim.master_board_id == Integer(BrainFlow.CYTON_BOARD)

    # for some special boards you need to provide the master_id in the other_info as string...
    params = BrainFlow.BrainFlowInputParams(other_info = "1")
    board_shim = BrainFlow.BoardShim(BrainFlow.STREAMING_BOARD, params)
    @test board_shim.master_board_id == 1

    # test the error if the other_info cannot be parsed as Int32
    params = BrainFlow.BrainFlowInputParams(other_info = "")
    @test_throws BrainFlow.BrainFlowError BrainFlow.BoardShim(BrainFlow.STREAMING_BOARD, params)
end

@testset "prepare_session error" begin
    # use a board that is not connected
    params = BrainFlow.BrainFlowInputParams()
    board_shim = BrainFlow.BoardShim(BrainFlow.CYTON_BOARD, params)
    @test_throws BrainFlow.BrainFlowError("Error in prepare_session INVALID_ARGUMENTS_ERROR", 13) BrainFlow.prepare_session(board_shim)
end

@testset "generated functions" begin
    @test BrainFlow.get_eeg_channels isa Function
end

@testset "data filtering" begin
    @test Int32(BrainFlow.CONSTANT) == 1
    @test Int32(BrainFlow.LINEAR) == 2

    @test Int32(BrainFlow.MEAN) == 0
    @test Int32(BrainFlow.MEDIAN) == 1
    @test Int32(BrainFlow.EACH) == 2

    @test Int32(BrainFlow.BUTTERWORTH) == 0
    @test Int32(BrainFlow.CHEBYSHEV_TYPE_1) == 1
    @test Int32(BrainFlow.BESSEL) == 2

    @test Int32(BrainFlow.NO_WINDOW) == 0
    @test Int32(BrainFlow.HANNING) == 1
    @test Int32(BrainFlow.HAMMING) == 2
    @test Int32(BrainFlow.BLACKMAN_HARRIS) == 3
end

@testset "model params" begin
    params = BrainFlowModelParams(metric = "concentration")
    @test params.metric == BrainFlow.Concentration()

    params = BrainFlowModelParams(metric = "Relaxation", classifier = "KNN")
    @test params.metric == BrainFlow.Relaxation()
    @test params.classifier == BrainFlow.Knn()

    json = JSON.json(BrainFlowModelParams())
    d = JSON.parse(json)
    @test d["classifier"] == 0
    @test d["metric"] == 0
    @test d["file"] == ""
    @test d["other_info"] == ""
end