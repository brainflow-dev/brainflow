import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--duration', type=int, required=False, default=10)
    parser.add_argument('--mac-address', type=str, required=False, default='')
    parser.add_argument('--serial-number', type=str, required=False, default='')
    parser.add_argument('--timeout', type=int, required=False, default=0)
    parser.add_argument('--output-prefix', type=str, required=False, default='muse_anthena')
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.mac_address = args.mac_address
    params.serial_number = args.serial_number
    params.timeout = args.timeout
    params.other_info = 'preset=p1041;low_latency=true'

    board_id = BoardIds.MUSE_S_ANTHENA_BOARD.value
    board = BoardShim(board_id, params)

    try:
        board.prepare_session()
        board.start_stream()
        try:
            time.sleep(args.duration)
            eeg_data = board.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
            accel_data = board.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
            optics_data = board.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)
        finally:
            board.stop_stream()
    finally:
        if board.is_prepared():
            board.release_session()

    DataFilter.write_file(eeg_data, f'{args.output_prefix}_eeg.csv', 'w')
    DataFilter.write_file(accel_data, f'{args.output_prefix}_accel_gyro.csv', 'w')
    DataFilter.write_file(optics_data, f'{args.output_prefix}_optics_battery.csv', 'w')


if __name__ == '__main__':
    main()
