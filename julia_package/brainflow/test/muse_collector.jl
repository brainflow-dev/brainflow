using BrainFlow

BrainFlow.enable_dev_logger(BrainFlow.BOARD_CONTROLLER)

params = BrainFlowInputParams()
board_shim = BrainFlow.BoardShim(BrainFlow.MUSE_S_BOARD, params)

BrainFlow.prepare_session(board_shim)
BrainFlow.config_board("p61", board_shim) # to enable ppg only use p61, p50 enables aux(5th eeg) channel, ppg and smth else
BrainFlow.add_streamer("file://default4.csv:w", board_shim, BrainFlow.DEFAULT_PRESET)
BrainFlow.add_streamer("file://aux4.csv:w", board_shim, BrainFlow.AUXILIARY_PRESET)
BrainFlow.add_streamer("file://anc4.csv:w", board_shim, BrainFlow.ANCILLARY_PRESET) # this preset contains ppg data and not available for Muse 2016
BrainFlow.start_stream(board_shim)
sleep(24000)
BrainFlow.stop_stream(board_shim)
BrainFlow.release_session(board_shim)