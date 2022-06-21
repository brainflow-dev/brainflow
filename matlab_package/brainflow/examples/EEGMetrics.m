BoardShim.set_log_file('brainflow.log');
MLModel.set_log_file('brainflow_ml.log');
BoardShim.enable_dev_board_logger();

params = BrainFlowInputParams();
board_shim = BoardShim(int32(BoardIds.SYNTHETIC_BOARD), params);
sampling_rate = BoardShim.get_sampling_rate(int32(BoardIds.SYNTHETIC_BOARD), 'default');
board_shim.prepare_session();
board_shim.start_stream(45000, '');
pause(5);
board_shim.stop_stream();
nfft = DataFilter.get_nearest_power_of_two(sampling_rate);
data = board_shim.get_board_data(board_shim.get_board_data_count('default'), 'default');
board_shim.release_session();

eeg_channels = BoardShim.get_eeg_channels(int32(BoardIds.SYNTHETIC_BOARD), 'default');
[avgs, stddevs] = DataFilter.get_avg_band_powers(data, eeg_channels, sampling_rate, true);
feature_vector = avgs;

model_params = BrainFlowModelParams(int32(BrainFlowMetrics.RESTFULNESS), int32(BrainFlowClassifiers.DEFAULT_CLASSIFIER));
model = MLModel(model_params);
model.prepare();
score = model.predict(feature_vector);
model.release();