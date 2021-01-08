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
wavelet_data <- brainflow_python$DataFilter$perform_wavelet_transform(numpy_data, "db4", as.integer(3))
restored_data <- brainflow_python$DataFilter$perform_inverse_wavelet_transform(wavelet_data, length(numpy_data), "db4", as.integer(3))
print(restored_data)