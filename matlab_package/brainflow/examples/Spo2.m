BoardShim.enable_dev_board_logger();
DataFilter.enable_dev_data_logger();
DataFilter.set_log_file('data.log');

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIds.SYNTHETIC_BOARD), params);
board_id = int32(BoardIds.SYNTHETIC_BOARD);
preset = int32(BrainFlowPresets.DEFAULT_PRESET);
board_descr = BoardShim.get_board_descr(board_id, preset);
sampling_rate = int32(board_descr.sampling_rate);
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(200);
board_shim.stop_stream();
nfft = DataFilter.get_nearest_power_of_two(sampling_rate);
data = board_shim.get_board_data(board_shim.get_board_data_count(preset), preset);
board_shim.release_session();

ppg_channels = board_descr.ppg_channels;
ir_channel = ppg_channels(1);
red_channel = ppg_channels(2);
ir_data = data(ir_channel, :);
red_data = data(red_channel, :);
spo2 = DataFilter.get_oxygen_level(ir_data, red_data, sampling_rate, 0.0, -37.663, 114.91);
heart_rate = DataFilter.get_heart_rate(ir_data, red_data, sampling_rate, 1024);