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
data_first_channel = data[eeg_channels[1], :]

# returns tuple of wavelet coeffs and lengths
wavelet_data = brainflow.perform_wavelet_transform(data_first_channel, "db4", 2)
restored_wavelet_data = brainflow.perform_inverse_wavelet_transform(wavelet_data, length(data_first_channel), "db4", 2)

fft_data = brainflow.perform_fft(data_first_channel)
restored_fft_data = brainflow.perform_ifft(fft_data)


println("Original Data")
println(data_first_channel)
println("Restored from Wavelet Data")
println(restored_wavelet_data)
println("Restored from FFT Data")
println(restored_fft_data)
