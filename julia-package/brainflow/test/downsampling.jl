import brainflow

# specify logging library to use
brainflow.enable_dev_brainflow_logger(brainflow.BOARD_CONTROLLER)

params = brainflow.BrainFlowInputParams()
board_shim = brainflow.BoardShim(brainflow.SYNTHETIC_BOARD, params)

brainflow.prepare_session(board_shim)
brainflow.start_stream(board_shim)
sleep(5)
brainflow.stop_stream(board_shim)
data = brainflow.get_current_board_data(32, board_shim)
brainflow.release_session(board_shim)

eeg_channels = brainflow.get_eeg_channels(brainflow.SYNTHETIC_BOARD)
sampling_rate = brainflow.get_sampling_rate(brainflow.SYNTHETIC_BOARD)

data_first_channel = data[eeg_channels[1], :]
println("Original Data First Channel")
println(data_first_channel)
downsampled_data = brainflow.perform_downsampling(data_first_channel, 3, Integer(brainflow.EACH))
println("After Downsampling")
println(downsampled_data)