BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIDs.SYNTHETIC_BOARD), params);
board_shim.prepare_session();
a = board_shim.config_board('~6');
board_shim.start_stream(45000, '');
pause(5);
board_shim.stop_stream();
data = board_shim.get_current_board_data(10);
disp(data);
board_shim.release_session();