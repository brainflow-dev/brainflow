import numpy

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds


def main():
    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.SYNTHETIC_BOARD, params)
    board.prepare_session()
    config = numpy.zeros(4).astype(numpy.ubyte)
    config[1] = 3
    board.config_board_with_bytes(config)
    board.release_session()


if __name__ == "__main__":
    main()
