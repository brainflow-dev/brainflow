BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIDs.SYNTHETIC_BOARD), params);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(2)
board_shim.stop_stream()
data = board_shim.get_current_board_data(20);
board_shim.release_session();

DataFilter.write_file(data, 'data.csv', 'w');
restored_data = DataFilter.read_file('data.csv');
