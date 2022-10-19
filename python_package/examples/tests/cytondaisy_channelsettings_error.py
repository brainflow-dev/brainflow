import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, WindowOperations, DetrendOperations


def main():
    BoardShim.enable_dev_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--serial-port', type=str, help='serial port', required=True)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.serial_port = args.serial_port

    data_streaming_seconds = 5
    board_id = BoardIds.CYTON_BOARD
    board_descr = BoardShim.get_board_descr(board_id)
    sampling_rate = int(board_descr['sampling_rate'])
    board = BoardShim(board_id, params)
    nfft = DataFilter.get_nearest_power_of_two(sampling_rate)
    eeg_channels = board_descr['eeg_channels']

    channel_chars = ["1", "2", "3", "4", "5", "6", "7", "8", "Q", "W", "E", "R", "T", "Y", "U", "I"]
    gain_chars = ["6", "5", "4", "3", "2", "1", "0"]
    input_type_char = "5"
    gain_test_all_channels = []
    board.release_all_sessions()

    # try to set gain for channel 9 for cyton without daisy and confirm error
    try:

            board.prepare_session()
            # Use 8 channels only
            resp = board.config_board("c")
            print(resp)
            # Send string to config a channel greater than 8
            config_string = f"x{channel_chars[15]}0{gain_chars[6]}{input_type_char}110X"
            resp = board.config_board(config_string)
            print(resp)
            # check that there is a response if streaming is off
            if not resp:
                raise ValueError('resp is None')
            board.start_stream()
            time.sleep(data_streaming_seconds)
            data_old = board.get_board_data()
            board.stop_stream()
            board.release_session()
            
    finally:
        if board.is_prepared():
            board.release_session()
            

if __name__ == "__main__":
    main()