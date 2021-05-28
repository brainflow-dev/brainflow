using BrainFlow

# specify logging library to use
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)
board_descr = BrainFlow.get_board_descr(BrainFlow.SYNTHETIC_BOARD)
sampling_rate = board_descr["sampling_rate"]
nfft = BrainFlow.get_nearest_power_of_two(sampling_rate)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(5)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_board_data(board_shim)
BrainFlow.release_session(board_shim)

eeg_channels = board_descr["eeg_channels"]
# second channel of synthetic board is sine wave at 10 Hz, should see huge 'alpha'
data_second_channel = data[eeg_channels[2], :]

# optional: detrend
BrainFlow.detrend(data_second_channel, BrainFlow.LINEAR)
# psd is a tuple of ampls and freqs
psd = BrainFlow.get_psd_welch(data_second_channel, nfft, Integer(nfft / 2), sampling_rate, BrainFlow.BLACKMAN_HARRIS)
band_power_alpha = BrainFlow.get_band_power(psd, 7.0, 13.0)
band_power_beta = BrainFlow.get_band_power(psd, 14.0, 30.0)
println(band_power_alpha / band_power_beta)