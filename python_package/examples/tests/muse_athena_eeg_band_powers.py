import argparse
import csv
import json
import time
from pathlib import Path

import numpy as np

from brainflow.board_shim import (
    BoardIds,
    BoardShim,
    BrainFlowInputParams,
    BrainFlowPresets,
    LogLevels,
)
from brainflow.data_filter import DataFilter, DetrendOperations, WindowOperations


BANDS = [
    ('delta', 2.0, 4.0),
    ('theta', 4.0, 8.0),
    ('alpha', 8.0, 13.0),
    ('beta', 13.0, 30.0),
    ('gamma', 30.0, 45.0),
]


def write_eeg_csv(path, eeg_data, eeg_channels):
    header = [f'eeg_row_{channel}' for channel in eeg_channels]
    np.savetxt(path, eeg_data.T, delimiter=',', header=','.join(header), comments='')


def calc_channel_band_powers(eeg_data, sampling_rate):
    nfft = DataFilter.get_nearest_power_of_two(sampling_rate)
    per_channel = []
    for channel_index in range(eeg_data.shape[0]):
        channel_data = np.copy(eeg_data[channel_index])
        DataFilter.detrend(channel_data, DetrendOperations.LINEAR.value)
        psd = DataFilter.get_psd_welch(
            channel_data,
            nfft,
            nfft // 2,
            sampling_rate,
            WindowOperations.BLACKMAN_HARRIS.value,
        )
        values = {
            name: DataFilter.get_band_power(psd, start, stop)
            for name, start, stop in BANDS
        }
        per_channel.append(values)
    return per_channel


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--timeout', type=int, required=False, default=30)
    parser.add_argument('--mac-address', type=str, required=False, default='')
    parser.add_argument('--serial-number', type=str, required=False, default='')
    parser.add_argument('--duration', type=float, required=False, default=30.0)
    parser.add_argument('--buffer-size', type=int, required=False, default=450000)
    parser.add_argument('--output-prefix', type=str, required=False, default='muse_athena_eeg')
    parser.add_argument('--no-filter', action='store_true')
    args = parser.parse_args()

    BoardShim.enable_dev_board_logger()
    board_id = BoardIds.MUSE_S_ATHENA_BOARD
    preset = BrainFlowPresets.DEFAULT_PRESET

    params = BrainFlowInputParams()
    params.timeout = args.timeout
    params.mac_address = args.mac_address
    params.serial_number = args.serial_number

    sampling_rate = BoardShim.get_sampling_rate(board_id, preset)
    eeg_channels = BoardShim.get_eeg_channels(board_id, preset)
    board_descr = BoardShim.get_board_descr(board_id, preset)

    prefix = Path(args.output_prefix)
    full_data_file = prefix.with_name(f'{prefix.name}_full_default.csv')
    eeg_data_file = prefix.with_name(f'{prefix.name}_eeg_only.csv')
    streamer_file = prefix.with_name(f'{prefix.name}_streamer_default.csv')
    summary_json_file = prefix.with_name(f'{prefix.name}_band_powers.json')
    per_channel_csv_file = prefix.with_name(f'{prefix.name}_band_powers_by_channel.csv')

    print(f'board_id={int(board_id)} preset={int(preset)}')
    print(f'sampling_rate={sampling_rate}')
    print(f'eeg_channels={eeg_channels}')
    print(f'eeg_names={board_descr.get("eeg_names", "")}')
    print(f'duration={args.duration}')

    board = BoardShim(board_id, params)
    prepared = False
    streaming = False
    try:
        board.prepare_session()
        prepared = True
        board.add_streamer(f'file://{streamer_file}:w', preset)
        board.start_stream(args.buffer_size)
        streaming = True
        BoardShim.log_message(LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
        time.sleep(args.duration)
        data = board.get_board_data(preset=preset)
    finally:
        if streaming:
            board.stop_stream()
        if prepared:
            board.release_session()

    if data.shape[1] < sampling_rate:
        raise RuntimeError(f'not enough data for band powers: {data.shape[1]} samples')

    eeg_data = data[eeg_channels, :]
    DataFilter.write_file(data, str(full_data_file), 'w')
    write_eeg_csv(eeg_data_file, eeg_data, eeg_channels)

    avg_bands, stddev_bands = DataFilter.get_avg_band_powers(
        data,
        eeg_channels,
        sampling_rate,
        not args.no_filter,
    )
    per_channel = calc_channel_band_powers(eeg_data, sampling_rate)

    summary = {
        'board_id': int(board_id),
        'preset': int(preset),
        'sampling_rate': int(sampling_rate),
        'samples': int(data.shape[1]),
        'duration_seconds': float(data.shape[1] / sampling_rate),
        'eeg_channels': [int(channel) for channel in eeg_channels],
        'eeg_names': board_descr.get('eeg_names', ''),
        'apply_filter': not args.no_filter,
        'avg_band_powers': {
            BANDS[i][0]: float(avg_bands[i])
            for i in range(len(BANDS))
        },
        'stddev_band_powers': {
            BANDS[i][0]: float(stddev_bands[i])
            for i in range(len(BANDS))
        },
        'files': {
            'full_default': str(full_data_file),
            'eeg_only': str(eeg_data_file),
            'streamer_default': str(streamer_file),
            'summary_json': str(summary_json_file),
            'per_channel_csv': str(per_channel_csv_file),
        },
    }

    with summary_json_file.open('w', encoding='utf-8') as f:
        json.dump(summary, f, indent=2)

    with per_channel_csv_file.open('w', newline='', encoding='utf-8') as f:
        fieldnames = ['channel_index', 'board_row'] + [band[0] for band in BANDS]
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for channel_index, powers in enumerate(per_channel):
            row = {
                'channel_index': channel_index,
                'board_row': eeg_channels[channel_index],
            }
            row.update(powers)
            writer.writerow(row)

    print(f'data_shape={data.shape}')
    print(f'full_default_file={full_data_file}')
    print(f'eeg_only_file={eeg_data_file}')
    print(f'streamer_file={streamer_file}')
    print(f'summary_json_file={summary_json_file}')
    print(f'per_channel_csv_file={per_channel_csv_file}')
    print('avg_band_powers=' + json.dumps(summary['avg_band_powers'], sort_keys=True))
    print('stddev_band_powers=' + json.dumps(summary['stddev_band_powers'], sort_keys=True))
    for channel_index, powers in enumerate(per_channel):
        print(f'channel_{channel_index}_row_{eeg_channels[channel_index]}=' +
              json.dumps(powers, sort_keys=True))


if __name__ == '__main__':
    main()
