using Test
using brainflow

@testset "prepare_session error" begin
    # use a board that is not connected
    params = brainflow.BrainFlowInputParams()
    board_shim = brainflow.BoardShim(brainflow.CYTON_BOARD, params)
    @test_throws brainflow.BrainFlowError("Error in prepare_session 13", 13) brainflow.prepare_session(board_shim)
end

@testset "generated functions" begin
    @test brainflow.get_eeg_channels isa Function
end