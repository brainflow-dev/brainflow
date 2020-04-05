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

eeg_channels = brainflow.get_eeg_channels(Integer(brainflow.SYNTHETIC_BOARD))
sampling_rate = brainflow.get_sampling_rate(Integer(brainflow.SYNTHETIC_BOARD))

data_first_channel = data[eeg_channels[1], :]
println("Original Data First Channel")
println(data_first_channel)
brainflow.perform_rolling_filter(data_first_channel, 3, Integer(brainflow.MEAN))
println("After Rolling Filter")
println(data_first_channel)

data_second_channel = data[eeg_channels[2], :]
println("Original Data Second Channel")
println(data_second_channel)
brainflow.perform_wavelet_denoising(data_second_channel, "db4", 2)
println("After Wavelet Denoising")
println(data_second_channel)
