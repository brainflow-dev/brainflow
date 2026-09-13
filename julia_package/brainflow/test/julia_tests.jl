using Test
using BrainFlow
using JSON

@testset "BoardShim constructor" begin
    # for most boards the master_id == the regular id
    params = BrainFlow.BrainFlowInputParams()
    board_shim = BrainFlow.BoardShim(BrainFlow.CYTON_BOARD, params)
    @test board_shim.master_board_id == Integer(BrainFlow.CYTON_BOARD)

    # for some special boards you need to provide the master_id in the other_info as string...
    params = BrainFlow.BrainFlowInputParams(master_board = Integer(BrainFlow.SYNTHETIC_BOARD))
    board_shim = BrainFlow.BoardShim(BrainFlow.STREAMING_BOARD, params)
    @test board_shim.master_board_id == -1
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

@testset "channel reference" begin
    data = [1.0 2.0; 3.0 4.0; 5.0 6.0]
    BrainFlow.reference(data, [1, 3], [1, 2])
    @test data == [-1.0 -1.0; 3.0 4.0; 3.0 3.0]

    @test_throws BrainFlow.BrainFlowError BrainFlow.reference(copy(data), Int[], [1])
    @test_throws BrainFlow.BrainFlowError BrainFlow.reference(copy(data), [1], Int[])
    @test_throws BrainFlow.BrainFlowError BrainFlow.reference(copy(data), [4], [1])
    @test_throws BrainFlow.BrainFlowError BrainFlow.reference(copy(data), [1], [4])
end

@testset "presets" begin
    presets = BrainFlow.get_board_presets(BrainFlow.CYTON_BOARD)
    @test presets[1] == Int32(BrainFlow.DEFAULT_PRESET)
end

@testset "model params" begin
    params = BrainFlowModelParams(BrainFlow.RESTFULNESS, BrainFlow.DEFAULT_CLASSIFIER)
    @test params.metric == BrainFlow.RESTFULNESS
    @test params.classifier == BrainFlow.DEFAULT_CLASSIFIER
end
