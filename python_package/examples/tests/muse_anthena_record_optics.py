import argparse
import time

import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter


def print_summary(data, optical_channels, timestamp_channel, sampling_rate):
    print(f'samples: {data.shape[1]}')

    timestamps = data[timestamp_channel, :]
    if timestamps.size > 1 and np.all(np.isfinite(timestamps)):
        diffs = np.diff(timestamps)
        diffs = diffs[diffs > 0]
        if diffs.size > 0:
            print(f'timestamp duration: {timestamps[-1] - timestamps[0]:.3f} sec')
            print(f'timestamp sampling rate: {1.0 / np.median(diffs):.2f} Hz')
    print(f'expected sampling rate: {sampling_rate} Hz')

    active_channels = []
    for channel in optical_channels:
        row = data[channel, :]
        finite = row[np.isfinite(row)]
        if finite.size == 0:
            continue
        if np.std(finite) > 1e-9:
            active_channels.append(channel)

    print(f'active optical channels: {active_channels}')


def main():
    BoardShim.enable_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--duration', type=int, required=False, default=60)
    parser.add_argument('--mac-address', type=str, required=False, default='')
    parser.add_argument('--serial-number', type=str, required=False, default='')
    parser.add_argument('--timeout', type=int, required=False, default=0)
    parser.add_argument('--other-info', type=str, required=False, default='preset=p1035;low_latency=true')
    parser.add_argument('--output-file', type=str, required=False, default='muse_anthena_optics_recording.csv')
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.mac_address = args.mac_address
    params.serial_number = args.serial_number
    params.timeout = args.timeout
    params.other_info = args.other_info

    board_id = BoardIds.MUSE_S_ANTHENA_BOARD.value
    preset = BrainFlowPresets.ANCILLARY_PRESET
    optical_channels = BoardShim.get_optical_channels(board_id, preset)
    timestamp_channel = BoardShim.get_timestamp_channel(board_id, preset)
    sampling_rate = BoardShim.get_sampling_rate(board_id, preset)

    board = BoardShim(board_id, params)
    try:
        board.prepare_session()
        board.start_stream()
        try:
            time.sleep(args.duration)
            data = board.get_board_data(preset=preset)
        finally:
            board.stop_stream()
    finally:
        if board.is_prepared():
            board.release_session()

    DataFilter.write_file(data, args.output_file, 'w')
    print(f'wrote raw ancillary data: {args.output_file}')
    print_summary(data, optical_channels, timestamp_channel, sampling_rate)


if __name__ == '__main__':
    main()
