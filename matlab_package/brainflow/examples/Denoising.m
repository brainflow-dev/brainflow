BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIds.SYNTHETIC_BOARD), params);
preset = int32(BrainFlowPresets.DEFAULT_PRESET);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(5);
board_shim.stop_stream();
data = board_shim.get_current_board_data(64, preset);
board_shim.release_session();

eeg_channels = BoardShim.get_eeg_channels(int32(BoardIds.SYNTHETIC_BOARD), preset);
% apply wavelet denoising to the first eeg channel %
first_eeg_channel = eeg_channels(1);
noisy_data = data(first_eeg_channel, :);
denoised_data = DataFilter.perform_wavelet_denoising(noisy_data, int32(WaveletTypes.DB3), 3, int32(WaveletDenoisingTypes.SURESHRINK), int32(ThresholdTypes.HARD), int32(WaveletExtensionTypes.SYMMETRIC), int32(NoiseEstimationLevelTypes.FIRST_LEVEL));