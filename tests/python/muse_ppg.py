import argparse
import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds


def main():
    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()

    board = BoardShim(BoardIds.MUSE_S_BOARD, params)
    board.prepare_session()
    board.config_board('p61')
    board.start_stream(45000, 'file://data.csv:w')
    time.sleep(10)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    print(data)


if __name__ == "__main__":
    main()
