BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIDs.SYNTHETIC_BOARD), params);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(5);
board_shim.stop_stream();
data = board_shim.get_current_board_data(64);
board_shim.release_session();

eeg_channels = BoardShim.get_eeg_channels(int32(BoardIDs.SYNTHETIC_BOARD));
% apply wavelet denoising to the first eeg channel %
first_eeg_channel = eeg_channels(1);
noisy_data = data(first_eeg_channel, :);
denoised_data = DataFilter.perform_wavelet_denoising(noisy_data, 'db4', 2);