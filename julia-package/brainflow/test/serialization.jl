using brainflow


# specify logging library to use
brainflow.enable_dev_brainflow_logger(brainflow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = brainflow.BoardShim(brainflow.SYNTHETIC_BOARD, params)

brainflow.prepare_session(board_shim)
brainflow.start_stream(board_shim)
sleep(5)
brainflow.stop_stream(board_shim)
data = brainflow.get_current_board_data(32, board_shim)
brainflow.release_session(board_shim)

brainflow.write_file(data, "test.csv", "w")
restored_data = brainflow.read_file("test.csv")

println("Original Data")
println(data)
println("Restored Data")
println(restored_data)
