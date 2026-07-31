import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()
    DataFilter.enable_dev_data_logger()

    heart_rate_fft_size = 2048

    parser = argparse.ArgumentParser(
        description='Record Muse 2 PPG data and calculate SpO2 and heart rate.')
    parser.add_argument(
        '--duration', type=float,
        help='recording duration in seconds; longer recordings may be needed for heart-rate calculation (default: 200)',
        required=False, default=200)
    parser.add_argument('--mac-address', type=str, help='mac address or macOS device UUID', required=False, default='')
    parser.add_argument('--serial-number', type=str, help='serial number or device identifier', required=False,
                        default='')
    parser.add_argument('--timeout', type=int, help='timeout for device discovery or connection', required=False,
                        default=0)
    parser.add_argument('--other-info', type=str, help='other info', required=False, default='')
    args = parser.parse_args()
    if args.duration <= 0:
        parser.error('--duration must be greater than zero')

    params = BrainFlowInputParams()
    params.mac_address = args.mac_address
    params.serial_number = args.serial_number
    params.timeout = args.timeout
    params.other_info = args.other_info

    board = BoardShim(BoardIds.MUSE_2_BOARD, params)
    try:
        board.prepare_session()
        board.config_board('p50')
        board.start_stream()
        try:
            time.sleep(args.duration)
            data = board.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)
        finally:
            board.stop_stream()
    finally:
        if board.is_prepared():
            board.release_session()

    print(data)

    ppg_channels = BoardShim.get_ppg_channels(BoardIds.MUSE_2_BOARD,  BrainFlowPresets.ANCILLARY_PRESET)
    sampling_rate = BoardShim.get_sampling_rate(BoardIds.MUSE_2_BOARD, BrainFlowPresets.ANCILLARY_PRESET)
    ppg_ir = data[ppg_channels[1]]
    ppg_red = data[ppg_channels[0]]
    oxygen_level = DataFilter.get_oxygen_level(ppg_ir, ppg_red, sampling_rate)
    print(f'SpO2: {oxygen_level}')

    num_samples = min(ppg_ir.size, ppg_red.size)
    if num_samples < heart_rate_fft_size:
        print(f'Heart rate unavailable: received {num_samples} samples; at least {heart_rate_fft_size} are required. '
              'Increase --duration.')
        return

    # 8192 and longer recording is recommended, in this test use 2048
    heart_rate = DataFilter.get_heart_rate(ppg_ir, ppg_red, sampling_rate, heart_rate_fft_size)
    print(f'Heart rate: {heart_rate} BPM')


if __name__ == "__main__":
    main()
