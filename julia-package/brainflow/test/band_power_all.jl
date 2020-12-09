using brainflow

# specify logging library to use
brainflow.enable_dev_brainflow_logger(brainflow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = brainflow.BoardShim(brainflow.SYNTHETIC_BOARD, params)
sampling_rate = brainflow.get_sampling_rate(brainflow.SYNTHETIC_BOARD)
nfft = brainflow.get_nearest_power_of_two(sampling_rate)

brainflow.prepare_session(board_shim)
brainflow.start_stream(board_shim)
sleep(5)
brainflow.stop_stream(board_shim)
data = brainflow.get_board_data(board_shim)
brainflow.release_session(board_shim)

eeg_channels = brainflow.get_eeg_channels(brainflow.SYNTHETIC_BOARD)

bands = brainflow.get_avg_band_powers(data, eeg_channels, sampling_rate, true)
println(bands[1])
println(bands[2])