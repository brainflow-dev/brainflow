import argparse
import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--ip-address', type=str, help='ip address', required=True)
    parser.add_argument('--ip-port', type=int, help='ip port', required=True)
    args = parser.parse_args()

    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()
    params.ip_address = args.ip_address;
    params.ip_port = args.ip_port;
    board = BoardShim(BoardIds.GANGLION_WIFI_BOARD.value, params)
    board.prepare_session()

    # 5 seconds of resistance data
    board.config_board('z')
    board.start_stream(45000, 'file://raw_data_resistance.csv:w')
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    # now get eeg data
    board.config_board('Z')
    board.start_stream(45000, 'file://raw_data_eeg.csv:w')
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    print(data)


if __name__ == "__main__":
    main()
