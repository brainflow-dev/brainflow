using BrainFlow

BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim, 45000, "file://data.csv:w")
sleep(1)
BrainFlow.insert_marker(1.0, board_shim)
sleep(1)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_current_board_data(256, board_shim)
BrainFlow.release_session(board_shim)

