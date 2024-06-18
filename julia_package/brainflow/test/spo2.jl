using BrainFlow

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)
board_descr = BrainFlow.get_board_descr(BrainFlow.SYNTHETIC_BOARD)
sampling_rate = board_descr["sampling_rate"]

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(5)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_board_data(board_shim)
BrainFlow.release_session(board_shim)

ppg_channels = board_descr["ppg_channels"]
data_ir = data[ppg_channels[1], :]
data_red = data[ppg_channels[2], :]

spo2 = BrainFlow.get_oxygen_level(data_ir, data_red, sampling_rate)
println(spo2)
