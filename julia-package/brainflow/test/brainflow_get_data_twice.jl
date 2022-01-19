using BrainFlow

# specify logging library to use
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(5)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_board_data(board_shim)
println(data)
data = BrainFlow.get_board_data(10, board_shim)
println(data)
data = BrainFlow.get_current_board_data(10, board_shim)
println(data)
BrainFlow.release_session(board_shim)
