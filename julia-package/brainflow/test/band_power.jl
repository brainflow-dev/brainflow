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
# second channel of synthetic board is sine wave at 10 Hz, should see huge 'alpha'
data_second_channel = data[eeg_channels[2], :]

# optional: detrend
brainflow.detrend(data_second_channel, brainflow.LINEAR)
# psd is a tuple of ampls and freqs
psd = brainflow.get_psd_welch(data_second_channel, nfft, Integer(nfft / 2), sampling_rate, brainflow.BLACKMAN_HARRIS)
band_power_alpha = brainflow.get_band_power(psd, 7.0, 13.0)
band_power_beta = brainflow.get_band_power(psd, 14.0, 30.0)
println(band_power_alpha / band_power_beta)