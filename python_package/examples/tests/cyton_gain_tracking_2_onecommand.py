import argparse
import time
import numpy as np
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, WindowOperations, DetrendOperations
def main():
    BoardShim.enable_dev_board_logger()
    parser = argparse.ArgumentParser()
    parser.add_argument('--serial-port', type=str, help='serial port', required=True)
    args = parser.parse_args()
    params = BrainFlowInputParams()
    params.serial_port = args.serial_port
    board_id = BoardIds.CYTON_BOARD
    board_descr = BoardShim.get_board_descr(board_id)
    sampling_rate = BoardShim.get_sampling_rate(board_id)
    eeg_channels = BoardShim.get_eeg_channels(board_id)
    board = BoardShim(board_id, params)
    board.prepare_session()
    config_string = "x1020000Xx2020000Xx3020000X"
    board.config_board(config_string)
    board.start_stream()
    time.sleep(5)
    data_old = board.get_board_data()
    board.stop_stream()
    board.release_session()
    board.prepare_session()
    config_string = "x1020000Xx2020000Xx3hiX"
    board.config_board(config_string)
    board.start_stream()
    time.sleep(5)
    data_new = board.get_board_data()
    board.stop_stream()
    board.release_session()
    print(np.mean(data_old[eeg_channels[2]][10:]))
    print(np.mean(data_new[eeg_channels[2]][10:]))
if __name__ == "__main__":
    main()