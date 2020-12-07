using brainflow


# specify logging library to use
brainflow.enable_dev_brainflow_logger(brainflow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = brainflow.BoardShim(brainflow.SYNTHETIC_BOARD, params)

brainflow.prepare_session(board_shim)
brainflow.start_stream(board_shim)
sleep(5)
brainflow.stop_stream(board_shim)
data = brainflow.get_current_board_data(32, board_shim)
brainflow.release_session(board_shim)

eeg_channels = brainflow.get_eeg_channels(brainflow.SYNTHETIC_BOARD)
sampling_rate = brainflow.get_sampling_rate(brainflow.SYNTHETIC_BOARD)

data_first_channel = data[eeg_channels[1], :]
println("Original Data First Channel")
println(data_first_channel)
brainflow.perform_lowpass(data_first_channel, sampling_rate, 10.0, 3, brainflow.BUTTERWORTH, 0.0)
println("After LowPass Filter")
println(data_first_channel)

data_second_channel = data[eeg_channels[2], :]
println("Original Data Second Channel")
println(data_second_channel)
brainflow.perform_highpass(data_second_channel, sampling_rate, 5.0, 3, brainflow.CHEBYSHEV_TYPE_1, 1.0)
println("After HighPass Filter")
println(data_second_channel)

data_third_channel = data[eeg_channels[3], :]
println("Original Data Third Channel")
println(data_third_channel)
brainflow.perform_bandpass(data_third_channel, sampling_rate, 25.0, 20.0, 3, brainflow.BESSEL, 0.0)
println("After BandPass Filter")
println(data_third_channel)

data_fourth_channel = data[eeg_channels[4], :]
println("Original Data Fourth Channel")
println(data_fourth_channel)
brainflow.perform_bandstop(data_fourth_channel, sampling_rate, 50.0, 2.0, 3, brainflow.BESSEL, 0.0)
println("After BandStop Filter")
println(data_fourth_channel)
