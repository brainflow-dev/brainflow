BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIds.SYNTHETIC_BOARD), params);
preset = int32(BrainFlowPresets.DEFAULT_PRESET);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(2);
board_shim.insert_marker(1, preset);
pause(2);
board_shim.stop_stream();
data = board_shim.get_board_data(board_shim.get_board_data_count(preset), preset);
disp(data);
board_shim.release_session();