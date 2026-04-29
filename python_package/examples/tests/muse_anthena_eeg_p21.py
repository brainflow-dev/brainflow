import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets


def main():
    BoardShim.enable_dev_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--duration', type=int, required=False, default=10)
    parser.add_argument('--mac-address', type=str, required=False, default='')
    parser.add_argument('--serial-number', type=str, required=False, default='')
    parser.add_argument('--timeout', type=int, required=False, default=0)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.mac_address = args.mac_address
    params.serial_number = args.serial_number
    params.timeout = args.timeout
    params.other_info = 'preset=p21;low_latency=true'

    board_id = BoardIds.MUSE_S_ANTHENA_BOARD.value
    board = BoardShim(board_id, params)

    try:
        board.prepare_session()
        board.start_stream()
        try:
            time.sleep(args.duration)
            eeg_data = board.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
            print(eeg_data)
        finally:
            board.stop_stream()
    finally:
        if board.is_prepared():
            board.release_session()


if __name__ == '__main__':
    main()
