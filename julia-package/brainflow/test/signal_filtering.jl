using BrainFlow


# specify logging library to use
BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.SYNTHETIC_BOARD, params)

BrainFlow.prepare_session(board_shim)
BrainFlow.start_stream(board_shim)
sleep(5)
BrainFlow.stop_stream(board_shim)
data = BrainFlow.get_current_board_data(32, board_shim)
BrainFlow.release_session(board_shim)

eeg_channels = BrainFlow.get_eeg_channels(BrainFlow.SYNTHETIC_BOARD)
sampling_rate = BrainFlow.get_sampling_rate(BrainFlow.SYNTHETIC_BOARD)

data_first_channel = data[eeg_channels[1], :]
println("Original Data First Channel")
println(data_first_channel)
BrainFlow.perform_lowpass(data_first_channel, sampling_rate, 10.0, 3, BrainFlow.BUTTERWORTH, 0.0)
println("After LowPass Filter")
println(data_first_channel)

data_second_channel = data[eeg_channels[2], :]
println("Original Data Second Channel")
println(data_second_channel)
BrainFlow.perform_highpass(data_second_channel, sampling_rate, 5.0, 3, BrainFlow.CHEBYSHEV_TYPE_1, 1.0)
println("After HighPass Filter")
println(data_second_channel)

data_third_channel = data[eeg_channels[3], :]
println("Original Data Third Channel")
println(data_third_channel)
BrainFlow.perform_bandpass(data_third_channel, sampling_rate, 25.0, 20.0, 3, BrainFlow.BESSEL, 0.0)
println("After BandPass Filter")
println(data_third_channel)

data_fourth_channel = data[eeg_channels[4], :]
println("Original Data Fourth Channel")
println(data_fourth_channel)
BrainFlow.perform_bandstop(data_fourth_channel, sampling_rate, 50.0, 2.0, 3, BrainFlow.BESSEL, 0.0)
println("After BandStop Filter")
println(data_fourth_channel)

data_fifth_channel = data[eeg_channels[5], :]
println("Original Data Fifth Channel")
println(data_fifth_channel)
BrainFlow.remove_environmental_noise(data_fifth_channel, sampling_rate, BrainFlow.FIFTY)
println("After BandStop Filter")
println(data_fifth_channel)
