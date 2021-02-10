import argparse
import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main():
    BoardShim.enable_dev_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--serial-port', type=str, help='serial port', required=True)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.serial_port = args.serial_port

    board = BoardShim(BoardIds.CYTON_BOARD, params)
    try:
        board.prepare_session()
        resp = board.config_board('?')
        print(resp)
        # check that there is a response if streaming is off
        if not resp:
            raise ValueError('resp is None')
        board.start_stream()
        time.sleep(5)
        resp = board.config_board('?')
        print(resp)
        # check that there is no response if streaming is on
        if resp:
            raise ValueError('resp is not None')
    finally:
        if board.is_prepared():
            board.release_session()


if __name__ == "__main__":
    main()
