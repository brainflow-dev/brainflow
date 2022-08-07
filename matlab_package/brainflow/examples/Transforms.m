BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIds.SYNTHETIC_BOARD), params);
preset = int32(BrainFlowPresets.DEFAULT_PRESET);
sampling_rate = BoardShim.get_sampling_rate(int32(BoardIds.SYNTHETIC_BOARD), preset);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(5);
board_shim.stop_stream();
data = board_shim.get_current_board_data(256, preset);
board_shim.release_session();

eeg_channels = BoardShim.get_eeg_channels(int32(BoardIds.SYNTHETIC_BOARD), preset);
% wavelet for first eeg channel %
first_eeg_channel = eeg_channels(1);
original_data = data(first_eeg_channel, :);
[wavelet_data, wavelet_lenghts] = DataFilter.perform_wavelet_transform(original_data, int32(WaveletTypes.DB3), 3, int32(WaveletExtensionTypes.SYMMETRIC));
restored_data = DataFilter.perform_inverse_wavelet_transform(wavelet_data, wavelet_lenghts, size(original_data, 2), int32(WaveletTypes.DB3), 3, int32(WaveletExtensionTypes.SYMMETRIC));
% fft for first eeg channel %
fft_data = DataFilter.perform_fft(original_data, int32(WindowOperations.NO_WINDOW));
restored_fft_data = DataFilter.perform_ifft(fft_data);