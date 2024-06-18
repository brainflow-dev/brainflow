from brainflow.board_shim import BoardShim

BoardShim.enable_dev_board_logger()

print(BoardShim.get_board_descr(0, "default"))
