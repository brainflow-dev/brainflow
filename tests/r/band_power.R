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

# need to convert to numpy array manually
numpy_data <- np$array(data[3,])
psd <- brainflow_python$DataFilter$get_psd_welch(numpy_data, as.integer(nfft), as.integer(nfft / 2),
    sampling_rate, brainflow_python$WindowFunctions$BLACKMAN_HARRIS$value)
band_power_alpha <- brainflow_python$DataFilter$get_band_power(psd, 7.0, 13.0)
band_power_beta <- brainflow_python$DataFilter$get_band_power(psd, 14.0, 30.0)
ratio <- band_power_alpha / band_power_beta