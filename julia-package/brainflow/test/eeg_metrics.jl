import brainflow


# enable all possible logs from all three libs
brainflow.enable_dev_brainflow_logger(Integer(brainflow.BOARD_CONTROLLER))
brainflow.enable_dev_brainflow_logger(Integer(brainflow.DATA_HANDLER))
brainflow.enable_dev_brainflow_logger(Integer(brainflow.ML_MODULE))

params = brainflow.BrainFlowInputParams()
board_shim = brainflow.BoardShim(Integer(brainflow.SYNTHETIC_BOARD), params)
sampling_rate = brainflow.get_sampling_rate(Integer(brainflow.SYNTHETIC_BOARD))
nfft = brainflow.get_nearest_power_of_two(sampling_rate)

brainflow.prepare_session(board_shim)
brainflow.start_stream(board_shim)
sleep(5)
brainflow.stop_stream(board_shim)
data = brainflow.get_board_data(board_shim)
brainflow.release_session(board_shim)

eeg_channels = brainflow.get_eeg_channels(Integer(brainflow.SYNTHETIC_BOARD))

bands = brainflow.get_avg_band_powers(data, eeg_channels, sampling_rate, true)
feature_vector = vcat(bands[1], bands[2])

# calc concentration
model_params = brainflow.BrainFlowModelParams(Integer(brainflow.CONCENTRATION), Integer(brainflow.KNN))
concentration = brainflow.MLModel(model_params)
brainflow.prepare(concentration)
print(brainflow.predict(feature_vector, concentration))
brainflow.release(concentration)

# calc relaxation
model_params = brainflow.BrainFlowModelParams(Integer(brainflow.RELAXATION), Integer(brainflow.REGRESSION))
relaxation = brainflow.MLModel(model_params)
brainflow.prepare(relaxation)
print(brainflow.predict(feature_vector, relaxation))
brainflow.release(relaxation)
