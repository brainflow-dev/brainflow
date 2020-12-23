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

eeg_channels = BrainFlow.get_eeg_channels(BrainFlow.SYNTHETIC_BOARD)
sampling_rate = BrainFlow.get_sampling_rate(BrainFlow.SYNTHETIC_BOARD)

data_first_channel = data[eeg_channels[1], :]
println("Original Data First Channel")
println(data_first_channel)
BrainFlow.perform_rolling_filter(data_first_channel, 3, BrainFlow.MEAN)
println("After Rolling Filter")
println(data_first_channel)

data_second_channel = data[eeg_channels[2], :]
println("Original Data Second Channel")
println(data_second_channel)
BrainFlow.perform_wavelet_denoising(data_second_channel, "db4", 2)
println("After Wavelet Denoising")
println(data_second_channel)
