using BrainFlow

# enable logs
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)
BrainFlow.enable_dev_logger(BrainFlow.DATA_HANDLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)
sampling_rate = BrainFlow.get_sampling_rate(BrainFlow.SYNTHETIC_BOARD)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(10)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_current_board_data(256, board_shim)
BrainFlow.release_session(board_shim)

eeg_channels = BrainFlow.get_eeg_channels(BrainFlow.SYNTHETIC_BOARD)
data_first_channel = data[eeg_channels[1], :]

# returns tuple of wavelet coeffs and lengths
wavelet_data = BrainFlow.perform_wavelet_transform(data_first_channel, BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)
restored_wavelet_data = BrainFlow.perform_inverse_wavelet_transform(wavelet_data, length(data_first_channel),
																	BrainFlow.DB3, 2, BrainFlow.SYMMETRIC)

fft_data = BrainFlow.perform_fft(data_first_channel, BrainFlow.NO_WINDOW)
restored_fft_data = BrainFlow.perform_ifft(fft_data)

println("Original Data")
println(data_first_channel)
println("Restored from Wavelet Data")
println(restored_wavelet_data)
println("Restored from FFT Data")
println(restored_fft_data)
