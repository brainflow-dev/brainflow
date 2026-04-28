import argparse
import math
import time

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets


def print_summary(optics_data, timestamps, sampling_rate, channel_labels):
    print(f'optics samples: {optics_data.shape[1]}')

    if timestamps.size > 1 and np.all(np.isfinite(timestamps)):
        timestamp_diffs = np.diff(timestamps)
        timestamp_diffs = timestamp_diffs[timestamp_diffs > 0]
        if timestamp_diffs.size > 0:
            actual_rate = 1.0 / np.median(timestamp_diffs)
            duration = timestamps[-1] - timestamps[0]
            print(f'timestamp duration: {duration:.3f} sec')
            print(f'timestamp sampling rate: {actual_rate:.2f} Hz, expected: {sampling_rate} Hz')

    nan_count = int(np.isnan(optics_data).sum())
    print(f'nan values: {nan_count}')

    for label, row in zip(channel_labels, optics_data):
        finite = row[np.isfinite(row)]
        if finite.size == 0:
            print(f'{label}: no finite values')
            continue
        stddev = float(np.std(finite))
        print(
            f'{label}: min={np.min(finite):.3f}, max={np.max(finite):.3f}, '
            f'mean={np.mean(finite):.3f}, std={stddev:.3f}'
        )
        if stddev < 1e-9:
            print(f'{label}: flat signal')


def plot_optics(optics_data, timestamps, sampling_rate, output_file, channel_labels):
    if timestamps.size == optics_data.shape[1] and timestamps.size > 0 and np.all(np.isfinite(timestamps)):
        x_axis = timestamps - timestamps[0]
        x_label = 'Time, sec'
    else:
        x_axis = np.arange(optics_data.shape[1]) / sampling_rate
        x_label = 'Samples'

    num_channels = optics_data.shape[0]
    cols = min(4, num_channels)
    rows = math.ceil(num_channels / cols)
    fig, axes = plt.subplots(rows, cols, figsize=(4 * cols, 2.4 * rows), sharex=True)
    axes = np.atleast_1d(axes).reshape(-1)

    for index, axis in enumerate(axes):
        if index < num_channels:
            axis.plot(x_axis, optics_data[index])
            axis.set_title(channel_labels[index])
            axis.grid(True)
        else:
            axis.axis('off')

    for axis in axes[-cols:]:
        axis.set_xlabel(x_label)

    fig.tight_layout()
    fig.savefig(output_file, dpi=150)
    print(f'wrote plot: {output_file}')


def main():
    BoardShim.enable_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--duration', type=int, required=False, default=20)
    parser.add_argument('--mac-address', type=str, required=False, default='')
    parser.add_argument('--serial-number', type=str, required=False, default='')
    parser.add_argument('--timeout', type=int, required=False, default=0)
    parser.add_argument('--other-info', type=str, required=False, default='preset=p1035;low_latency=true')
    parser.add_argument('--output-file', type=str, required=False, default='muse_anthena_optics_p1035.png')
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

    optics_data = data[optical_channels, :]
    timestamps = data[timestamp_channel, :]
    channel_labels = [f'Optics {channel}' for channel in optical_channels]
    print_summary(optics_data, timestamps, sampling_rate, channel_labels)
    plot_optics(optics_data, timestamps, sampling_rate, args.output_file, channel_labels)


if __name__ == '__main__':
    main()
