using BrainFlow


# specify logging library to use
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(5)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_current_board_data(32, board_shim)
BrainFlow.release_session(board_shim)

BrainFlow.write_file(data, "test.csv", "w")
restored_data = BrainFlow.read_file("test.csv")

println("Original Data")
println(data)
println("Restored Data")
println(restored_data)
