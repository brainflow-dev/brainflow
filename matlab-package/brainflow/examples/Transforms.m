BoardShim.set_log_file ('brainflow.log');
BoardShim.enable_dev_board_logger ();

params = BrainFlowInputParams ();
board_shim = BoardShim (int32 (BoardIDs.SYNTHETIC_BOARD), params);
board_shim.prepare_session ();
board_shim.start_stream (45000, '');
pause (5);
board_shim.stop_stream ();
data = board_shim.get_current_board_data (64);
board_shim.release_session ();

eeg_channels = BoardShim.get_eeg_channels (int32 (BoardIDs.SYNTHETIC_BOARD));
% wavelet for first eeg channel %
first_eeg_channel = eeg_channels (1);
original_data = data (first_eeg_channel, :);
[wavelet_data, wavelet_lenghts] = DataFilter.perform_wavelet_transform (original_data, 'db4', 2);
restored_data = DataFilter.perform_inverse_wavelet_transform (wavelet_data, wavelet_lenghts, size (original_data, 2), 'db4', 2);
% fft for first eeg channel %
fft_data = DataFilter.perform_fft (original_data);
restored_fft_data = DataFilter.perform_ifft (fft_data);