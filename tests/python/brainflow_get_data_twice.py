import argparse
import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main():
    BoardShim.enable_dev_board_logger()

    board = BoardShim(BoardIds.SYNTHETIC_BOARD.value, BrainFlowInputParams())
    board.prepare_session()

    board.start_stream ()
    time.sleep(10)
    board.stop_stream()
    data = board.get_board_data()
    data = board.get_board_data()
    print(data)
    data = board.get_current_board_data(10)
    print(data)
    board.release_session()


if __name__ == "__main__":
    main()
