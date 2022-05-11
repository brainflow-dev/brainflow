BoardShim.set_log_file('brainflow.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIDs.SYNTHETIC_BOARD), params);
sampling_rate = BoardShim.get_sampling_rate(int32(BoardIDs.SYNTHETIC_BOARD));
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(5);
board_shim.stop_stream();
nfft = DataFilter.get_nearest_power_of_two(sampling_rate);
data = board_shim.get_board_data(board_shim.get_board_data_count());
board_shim.release_session();

eeg_channels = BoardShim.get_eeg_channels(int32(BoardIDs.SYNTHETIC_BOARD));
[avgs, stddevs] = DataFilter.get_avg_band_powers(data, eeg_channels, sampling_rate, true);
feature_vector = double([avgs, stddevs]);

concentration_params = BrainFlowModelParams(int32(BrainFlowMetrics.CONCENTRATION), int32(BrainFlowClassifiers.REGRESSION));
concentration = MLModel(concentration_params);
concentration.prepare();
score = concentration.predict(feature_vector);
concentration.release();