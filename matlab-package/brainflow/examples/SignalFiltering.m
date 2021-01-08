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
% apply iir filter to the first eeg channel %
first_eeg_channel = eeg_channels(1);
original_data = data(first_eeg_channel, :);
sampling_rate = BoardShim.get_sampling_rate(int32(BoardIDs.SYNTHETIC_BOARD));
filtered_data = DataFilter.perform_lowpass(original_data, sampling_rate, 10.0, 3, int32(FilterTypes.BUTTERWORTH), 0.0);