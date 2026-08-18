library(brainflow)

params <- brainflow_python$BrainFlowInputParams()
board_shim <- brainflow_python$BoardShim(brainflow_python$BoardIds$SYNTHETIC_BOARD$value, params)
board_shim$prepare_session()
board_shim$start_stream()
Sys.sleep(time = 5)
board_shim$stop_stream()
data <- board_shim$get_current_board_data(as.integer(250))
board_shim$release_session()

# Re-reference every EEG channel using the sample-wise mean of the first two EEG channels.
# R delegates DataFilter operations to the Python binding, so keep the matrix as a NumPy array.
numpy_matrix <- np$array(data)
eeg_channels <- brainflow_python$BoardShim$get_eeg_channels(
    brainflow_python$BoardIds$SYNTHETIC_BOARD$value)
if (length(eeg_channels) >= 2)
{
    brainflow_python$DataFilter$reference(
        numpy_matrix, eeg_channels, eeg_channels[1:2])
}

numpy_data <- np$array(numpy_matrix[2,])
print(numpy_data)
brainflow_python$DataFilter$perform_downsampling(numpy_data, as.integer(3), brainflow_python$AggOperations$EACH$value)
print(numpy_data)
