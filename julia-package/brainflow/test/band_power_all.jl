using BrainFlow

# specify logging library to use
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)
sampling_rate = BrainFlow.get_sampling_rate(BrainFlow.SYNTHETIC_BOARD)
nfft = BrainFlow.get_nearest_power_of_two(sampling_rate)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(5)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_board_data(board_shim)
BrainFlow.release_session(board_shim)

eeg_channels = BrainFlow.get_eeg_channels(BrainFlow.SYNTHETIC_BOARD)

bands = BrainFlow.get_avg_band_powers(data, eeg_channels, sampling_rate, true)
println(bands[1])
println(bands[2])