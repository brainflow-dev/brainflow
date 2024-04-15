params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIds.SYNTHETIC_BOARD), params);
preset = int32(BrainFlowPresets.DEFAULT_PRESET);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(10);
board_shim.stop_stream();
data = board_shim.get_board_data(500, preset);
board_shim.release_session();

eeg_channels = BoardShim.get_eeg_channels(int32(BoardIds.SYNTHETIC_BOARD), preset);
selected_channel = eeg_channels(4);
original_data = data(selected_channel, :);
original_data = transpose(reshape(original_data, [100, 5]));
[w,k,a,s] = DataFilter.perform_ica(original_data, 2);