using brainflow

# enable all possible logs from all three libs
brainflow.enable_dev_brainflow_logger(brainflow.BOARD_CONTROLLER)
brainflow.enable_dev_brainflow_logger(brainflow.DATA_HANDLER)
brainflow.enable_dev_brainflow_logger(brainflow.ML_MODULE)

params = BrainFlowInputParams()
board_shim = brainflow.BoardShim(brainflow.SYNTHETIC_BOARD, params)
sampling_rate = brainflow.get_sampling_rate(brainflow.SYNTHETIC_BOARD)
nfft = brainflow.get_nearest_power_of_two(sampling_rate)

brainflow.prepare_session(board_shim)
brainflow.start_stream(board_shim)
sleep(5)
brainflow.stop_stream(board_shim)
data = brainflow.get_board_data(board_shim)
brainflow.release_session(board_shim)

eeg_channels = brainflow.get_eeg_channels(brainflow.SYNTHETIC_BOARD)

bands = brainflow.get_avg_band_powers(data, eeg_channels, sampling_rate, true)
feature_vector = vcat(bands[1], bands[2])

# calc concentration
model_params = BrainFlowModelParams(metric = "concentration", classifier = "KNN")
brainflow.prepare(model_params)
print(brainflow.predict(feature_vector, model_params))
brainflow.release(model_params)

# calc relaxation
model_params = BrainFlowModelParams(metric = "relaxation", classifier = "regression")
brainflow.prepare(model_params)
print(brainflow.predict(feature_vector, model_params))
brainflow.release(model_params)
