from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds

BoardShim.enable_dev_board_logger()


params = BrainFlowInputParams()
params.mac_address = "00:13:43:A1:84:EE"
board = BoardShim(BoardIds.EXPLORE_PLUS_8_CHAN_BOARD, params)
board.prepare_session()
board.start_stream ()
time.sleep(10)
# data = board.get_current_board_data (256) # get latest 256 packages or less, doesnt remove them from internal buffer
data = board.get_board_data()  # get all data and remove it from internal buffer
board.stop_stream()
board.release_session()

print(data)