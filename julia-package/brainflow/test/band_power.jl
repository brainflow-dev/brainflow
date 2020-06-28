import brainflow


brainflow.enable_dev_board_logger()

params = brainflow.BrainFlowInputParams()
board_shim = brainflow.BoardShim(Integer(brainflow.SYNTHETIC_BOARD), params)
sampling_rate = brainflow.get_sampling_rate(Integer(brainflow.SYNTHETIC_BOARD))

brainflow.prepare_session(board_shim)
brainflow.start_stream(board_shim)
sleep(5)
brainflow.stop_stream(board_shim)
data = brainflow.get_current_board_data(brainflow.get_nearest_power_of_two(sampling_rate), board_shim)
brainflow.release_session(board_shim)

eeg_channels = brainflow.get_eeg_channels(Integer(brainflow.SYNTHETIC_BOARD))
data_first_channel = data[eeg_channels[1], :]

# psd is a tuple of ampls and freqs
psd = brainflow.get_psd(data_first_channel, sampling_rate, Integer(brainflow.BLACKMAN_HARRIS))
band_power_alpha = brainflow.get_band_power(psd, 7.0, 13.0)
band_power_beta = brainflow.get_band_power(psd, 14.0, 30.0)
println(band_power_alpha / band_power_beta)