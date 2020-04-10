import brainflow


brainflow.enable_dev_board_logger()

params = brainflow.BrainFlowInputParams()
board_shim = brainflow.BoardShim(Integer(brainflow.SYNTHETIC_BOARD), params)

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
