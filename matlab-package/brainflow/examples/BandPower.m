BoardShim.set_log_file ('brainflow.log');
BoardShim.enable_dev_board_logger ();

params = BrainFlowInputParams ();
board_shim = BoardShim (int32 (BoardIDs.SYNTHETIC_BOARD), params);
sampling_rate = BoardShim.get_sampling_rate (int32 (BoardIDs.SYNTHETIC_BOARD));
board_shim.prepare_session ();
board_shim.start_stream (45000, '');
pause (5);
board_shim.stop_stream ();
data = board_shim.get_current_board_data (DataFilter.get_nearest_power_of_two (sampling_rate));
board_shim.release_session ();

eeg_channels = BoardShim.get_eeg_channels (int32 (BoardIDs.SYNTHETIC_BOARD));
first_eeg_channel = eeg_channels (1);
original_data = data (first_eeg_channel, :);
[ampls, freqs] = DataFilter.get_psd (original_data, sampling_rate, int32 (WindowFunctions.HAMMING));
band_power_alpha = DataFilter.get_band_power (ampls, freqs, 7.0, 13.0);
band_power_beta = DataFilter.get_band_power (ampls, freqs, 14.0, 30.0);
ratio = band_power_alpha / band_power_beta;