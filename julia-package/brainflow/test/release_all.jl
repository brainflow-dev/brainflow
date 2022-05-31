using BrainFlow

# enable all possible logs from all three libs
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)
BrainFlow.enable_dev_logger(BrainFlow.DATA_HANDLER)
BrainFlow.enable_dev_logger(BrainFlow.ML_MODULE)

params1 = BrainFlowInputParams()
params1.other_info = "board1"
board_shim1 = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params1)
params2 = BrainFlowInputParams()
params2.other_info = "board2"
board_shim2 = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params2)

BrainFlow.prepare_session(board_shim1)
BrainFlow.prepare_session(board_shim2)
BrainFlow.start_stream(board_shim1)
sleep(2)
data = BrainFlow.get_current_board_data(10, board_shim1)
println(BrainFlow.calc_stddev(data[2, :]))

BrainFlow.release_all_sessions()

# calc concentration
model_params = BrainFlowModelParams(BrainFlow.RESTFULNESS, BrainFlow.DEFAULT_CLASSIFIER)
BrainFlow.prepare(model_params)
BrainFlow.release_all()
