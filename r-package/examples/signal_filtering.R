library(brainflow)

params <- brainflow_python$BrainFlowInputParams()
board_shim <- brainflow_python$BoardShim(brainflow_python$BoardIds$SYNTHETIC_BOARD$value, params)
board_shim$prepare_session()
board_shim$start_stream()
Sys.sleep(time = 5)
board_shim$stop_stream()
data <- board_shim$get_current_board_data(as.integer(250))
board_shim$release_session()

# need to convert to numpy array manually
numpy_data <- np$array(data[2,])
print(numpy_data)
sampling_rate <- board_shim$get_sampling_rate(brainflow_python$BoardIds$SYNTHETIC_BOARD$value)
brainflow_python$DataFilter$perform_bandpass(numpy_data, sampling_rate, 10.0, 5.0, as.integer(3), brainflow_python$FilterTypes$BESSEL$value, 0)
print(numpy_data)

