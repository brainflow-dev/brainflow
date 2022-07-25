using BrainFlow

BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.MUSE_S_BOARD, params)

BrainFlow.prepare_session(board_shim)
BrainFlow.config_board("p50", board_shim) # to enable ppg only use p61, p50 enables aux(5th eeg) channel, ppg and smth else
BrainFlow.add_streamer("file://default_from_streamer.csv:w", board_shim, BrainFlow.DEFAULT_PRESET)
BrainFlow.add_streamer("file://aux_from_streamer.csv:w", board_shim, BrainFlow.AUXILIARY_PRESET)
BrainFlow.add_streamer("file://anc_from_streamer.csv:w", board_shim, BrainFlow.ANCILLARY_PRESET) # this preset contains ppg data and not available for Muse 2016
BrainFlow.start_stream(board_shim)
sleep(10)
BrainFlow.stop_stream(board_shim)
data_default = BrainFlow.get_board_data(board_shim, BrainFlow.DEFAULT_PRESET) # contains eeg data
data_aux = BrainFlow.get_board_data(board_shim, BrainFlow.AUXILIARY_PRESET) # contains accel and gyro data
data_anc = BrainFlow.get_board_data(board_shim, BrainFlow.ANCILLARY_PRESET) # contains ppg data
BrainFlow.release_session(board_shim)

BrainFlow.write_file(data_default, "default.csv", "w")
BrainFlow.write_file(data_aux, "aux.csv", "w")
BrainFlow.write_file(data_anc, "anc.csv", "w")
