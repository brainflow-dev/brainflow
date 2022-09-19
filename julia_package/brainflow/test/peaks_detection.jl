using BrainFlow

# enable logs
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)
BrainFlow.enable_dev_logger(BrainFlow.DATA_HANDLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)
sampling_rate = BrainFlow.get_sampling_rate(BrainFlow.SYNTHETIC_BOARD)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(10)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_current_board_data(1024, board_shim)
BrainFlow.release_session(board_shim)

eeg_channels = BrainFlow.get_eeg_channels(BrainFlow.SYNTHETIC_BOARD)
data_first_channel = data[eeg_channels[1], :]

restored_data = BrainFlow.restore_data_from_wavelet_detailed_coeffs(data_first_channel, BrainFlow.DB4, 4, 2)
peaks = BrainFlow.detect_peaks_z_score(restored_data, 20, 3.5, 0.0)