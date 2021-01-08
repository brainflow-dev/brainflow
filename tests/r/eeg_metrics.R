library(brainflow)

board_id <- brainflow_python$BoardIds$SYNTHETIC_BOARD$value
sampling_rate <- brainflow_python$BoardShim$get_sampling_rate(board_id)
nfft <- brainflow_python$DataFilter$get_nearest_power_of_two(sampling_rate)
params <- brainflow_python$BrainFlowInputParams()
board_shim <- brainflow_python$BoardShim(board_id, params)
board_shim$prepare_session()
board_shim$start_stream()
Sys.sleep(time = 10)
board_shim$stop_stream()
data <- board_shim$get_board_data()
board_shim$release_session()

eeg_channels <- brainflow_python$BoardShim$get_eeg_channels(board_id)
bands <- brainflow_python$DataFilter$get_avg_band_powers(data, eeg_channels, sampling_rate, TRUE)
feature_vector <- np$array(c(bands[[1]], bands[[2]]))

concentration_params <- brainflow_python$BrainFlowModelParams(brainflow_python$BrainFlowMetrics$CONCENTRATION$value, brainflow_python$BrainFlowClassifiers$REGRESSION$value)
concentration <- brainflow_python$MLModel(concentration_params)
concentration$prepare()
score <- concentration$predict(feature_vector)
concentration$release()