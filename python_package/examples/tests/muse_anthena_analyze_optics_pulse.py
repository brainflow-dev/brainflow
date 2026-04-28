import argparse
import math
from pathlib import Path

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from brainflow.board_shim import BoardShim, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter, DetrendOperations, FilterTypes


def build_time_axis(data, timestamp_channel, sampling_rate):
    timestamps = data[timestamp_channel, :]
    if timestamps.size == data.shape[1] and timestamps.size > 1 and np.all(np.isfinite(timestamps)):
        diffs = np.diff(timestamps)
        if np.count_nonzero(diffs > 0) > timestamps.size * 0.8:
            return timestamps - timestamps[0], 1.0 / np.median(diffs[diffs > 0])
    return np.arange(data.shape[1], dtype=np.float64) / sampling_rate, float(sampling_rate)


def get_active_channels(optics_data, optical_channels):
    active = []
    active_labels = []
    for index, channel in enumerate(optical_channels):
        row = optics_data[index, :]
        finite = row[np.isfinite(row)]
        if finite.size > 0 and np.std(finite) > 1e-9:
            active.append(index)
            active_labels.append(f'Optics {channel}')
    return active, active_labels


def zscore(data):
    stddev = np.std(data)
    if stddev < 1e-12:
        return np.zeros(data.shape)
    return (data - np.mean(data)) / stddev


def filter_active_channels(optics_data, active_indexes, sampling_rate, low_cut, high_cut):
    filtered = []
    for index in active_indexes:
        row = np.asarray(optics_data[index, :], dtype=np.float64).copy()
        finite = np.isfinite(row)
        if not np.all(finite):
            row[~finite] = np.interp(np.flatnonzero(~finite), np.flatnonzero(finite), row[finite])
        DataFilter.detrend(row, DetrendOperations.LINEAR.value)
        DataFilter.perform_bandpass(
            row,
            int(round(sampling_rate)),
            low_cut,
            high_cut,
            4,
            FilterTypes.BUTTERWORTH_ZERO_PHASE.value,
            0.0,
        )
        filtered.append(row)
    return np.asarray(filtered)


def find_local_peaks(signal, sampling_rate, min_bpm, max_bpm):
    if signal.size < 3:
        return np.asarray([], dtype=np.int64)

    min_distance = max(1, int(round(sampling_rate * 60.0 / max_bpm)))
    threshold = np.percentile(signal, 65.0)
    candidates = np.flatnonzero((signal[1:-1] > signal[:-2]) & (signal[1:-1] >= signal[2:])) + 1
    candidates = candidates[signal[candidates] > threshold]

    peaks = []
    for candidate in candidates:
        if not peaks or candidate - peaks[-1] >= min_distance:
            peaks.append(int(candidate))
        elif signal[candidate] > signal[peaks[-1]]:
            peaks[-1] = int(candidate)
    peaks = np.asarray(peaks, dtype=np.int64)

    if peaks.size < 3:
        return peaks

    intervals = np.diff(peaks) / sampling_rate
    valid = (intervals >= 60.0 / max_bpm) & (intervals <= 60.0 / min_bpm)
    keep = np.concatenate(([True], valid))
    return peaks[keep]


def score_peaks(peaks, signal, sampling_rate, min_bpm, max_bpm):
    if peaks.size < 3:
        return -math.inf
    intervals = np.diff(peaks) / sampling_rate
    valid = (intervals >= 60.0 / max_bpm) & (intervals <= 60.0 / min_bpm)
    if np.count_nonzero(valid) < 2:
        return -math.inf
    valid_intervals = intervals[valid]
    rr_cv = np.std(valid_intervals) / np.mean(valid_intervals)
    return float(np.mean(signal[peaks]) - rr_cv)


def select_pulse_signal(combined, sampling_rate, min_bpm, max_bpm):
    best_signal = combined
    best_peaks = find_local_peaks(combined, sampling_rate, min_bpm, max_bpm)
    best_score = score_peaks(best_peaks, combined, sampling_rate, min_bpm, max_bpm)

    inverted = -combined
    inverted_peaks = find_local_peaks(inverted, sampling_rate, min_bpm, max_bpm)
    inverted_score = score_peaks(inverted_peaks, inverted, sampling_rate, min_bpm, max_bpm)

    if inverted_score > best_score:
        best_signal = inverted
        best_peaks = inverted_peaks

    return best_signal, best_peaks


def bpm_from_peaks(peaks, time_axis):
    if peaks.size < 3:
        return None
    intervals = np.diff(time_axis[peaks])
    intervals = intervals[intervals > 0]
    if intervals.size == 0:
        return None
    return 60.0 / np.median(intervals)


def spectrum(signal, sampling_rate, low_cut, high_cut):
    if signal.size < 4:
        return np.asarray([]), np.asarray([]), None
    centered = signal - np.mean(signal)
    windowed = centered * np.hanning(centered.size)
    freqs = np.fft.rfftfreq(windowed.size, d=1.0 / sampling_rate)
    power = np.abs(np.fft.rfft(windowed)) ** 2
    band = (freqs >= low_cut) & (freqs <= high_cut)
    if not np.any(band):
        return freqs, power, None
    peak_freq = freqs[band][np.argmax(power[band])]
    return freqs, power, peak_freq * 60.0


def save_raw_plot(time_axis, optics_data, active_indexes, labels, output_file):
    fig, axes = plt.subplots(len(active_indexes), 1, figsize=(12, 2.1 * len(active_indexes)), sharex=True)
    axes = np.atleast_1d(axes)
    for axis, index, label in zip(axes, active_indexes, labels):
        axis.plot(time_axis, optics_data[index, :], linewidth=1.0)
        axis.set_ylabel(label)
        axis.grid(True)
    axes[-1].set_xlabel('Time, sec')
    fig.tight_layout()
    fig.savefig(output_file, dpi=150)
    plt.close(fig)


def save_filtered_plot(time_axis, filtered, labels, output_file):
    fig, axes = plt.subplots(filtered.shape[0], 1, figsize=(12, 2.1 * filtered.shape[0]), sharex=True)
    axes = np.atleast_1d(axes)
    for axis, row, label in zip(axes, filtered, labels):
        axis.plot(time_axis, zscore(row), linewidth=1.0)
        axis.set_ylabel(label)
        axis.grid(True)
    axes[-1].set_xlabel('Time, sec')
    fig.tight_layout()
    fig.savefig(output_file, dpi=150)
    plt.close(fig)


def save_pulse_plot(time_axis, pulse_signal, peaks, peak_bpm, spectrum_bpm, output_file):
    title_parts = []
    if peak_bpm is not None:
        title_parts.append(f'peaks {peak_bpm:.1f} BPM')
    if spectrum_bpm is not None:
        title_parts.append(f'spectrum {spectrum_bpm:.1f} BPM')

    fig, axis = plt.subplots(1, 1, figsize=(12, 4))
    axis.plot(time_axis, pulse_signal, linewidth=1.0)
    if peaks.size > 0:
        axis.plot(time_axis[peaks], pulse_signal[peaks], 'ro', markersize=3)
    axis.set_xlabel('Time, sec')
    axis.set_ylabel('Combined filtered optics, z-score')
    axis.set_title(', '.join(title_parts) if title_parts else 'Combined filtered optics')
    axis.grid(True)
    fig.tight_layout()
    fig.savefig(output_file, dpi=150)
    plt.close(fig)


def save_spectrum_plot(freqs, power, spectrum_bpm, output_file):
    fig, axis = plt.subplots(1, 1, figsize=(10, 4))
    if freqs.size > 0:
        axis.plot(freqs * 60.0, power, linewidth=1.0)
    if spectrum_bpm is not None:
        axis.axvline(spectrum_bpm, color='r', linestyle='--', linewidth=1.0)
    axis.set_xlabel('Frequency, BPM')
    axis.set_ylabel('Power')
    axis.set_xlim(30, 210)
    axis.grid(True)
    fig.tight_layout()
    fig.savefig(output_file, dpi=150)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input-file', type=str, required=False, default='muse_anthena_optics_recording.csv')
    parser.add_argument('--output-prefix', type=str, required=False, default='')
    parser.add_argument('--discard-seconds', type=float, required=False, default=5.0)
    parser.add_argument('--low-cut', type=float, required=False, default=0.7)
    parser.add_argument('--high-cut', type=float, required=False, default=3.5)
    parser.add_argument('--min-bpm', type=float, required=False, default=40.0)
    parser.add_argument('--max-bpm', type=float, required=False, default=180.0)
    args = parser.parse_args()

    data = DataFilter.read_file(args.input_file)
    board_id = BoardIds.MUSE_S_ANTHENA_BOARD.value
    preset = BrainFlowPresets.ANCILLARY_PRESET
    optical_channels = BoardShim.get_optical_channels(board_id, preset)
    timestamp_channel = BoardShim.get_timestamp_channel(board_id, preset)
    expected_sampling_rate = BoardShim.get_sampling_rate(board_id, preset)

    time_axis, actual_sampling_rate = build_time_axis(data, timestamp_channel, expected_sampling_rate)
    start_index = int(np.searchsorted(time_axis, args.discard_seconds))
    if start_index >= data.shape[1] - 4:
        start_index = 0
    data = data[:, start_index:]
    time_axis = time_axis[start_index:] - time_axis[start_index]

    optics_data = data[optical_channels, :]
    active_indexes, active_labels = get_active_channels(optics_data, optical_channels)
    if not active_indexes:
        raise RuntimeError('no active optical channels found')

    filtered = filter_active_channels(
        optics_data,
        active_indexes,
        actual_sampling_rate,
        args.low_cut,
        args.high_cut,
    )
    combined = np.mean(np.asarray([zscore(row) for row in filtered]), axis=0)
    pulse_signal, peaks = select_pulse_signal(combined, actual_sampling_rate, args.min_bpm, args.max_bpm)
    peak_bpm = bpm_from_peaks(peaks, time_axis)
    freqs, power, spectrum_bpm = spectrum(pulse_signal, actual_sampling_rate, args.low_cut, args.high_cut)

    prefix = args.output_prefix or str(Path(args.input_file).with_suffix(''))
    raw_plot = f'{prefix}_raw.png'
    filtered_plot = f'{prefix}_filtered.png'
    pulse_plot = f'{prefix}_pulse.png'
    spectrum_plot = f'{prefix}_spectrum.png'

    save_raw_plot(time_axis, optics_data, active_indexes, active_labels, raw_plot)
    save_filtered_plot(time_axis, filtered, active_labels, filtered_plot)
    save_pulse_plot(time_axis, pulse_signal, peaks, peak_bpm, spectrum_bpm, pulse_plot)
    save_spectrum_plot(freqs, power, spectrum_bpm, spectrum_plot)

    print(f'input file: {args.input_file}')
    print(f'samples analyzed: {data.shape[1]}')
    print(f'duration analyzed: {time_axis[-1] - time_axis[0]:.3f} sec')
    print(f'sampling rate: {actual_sampling_rate:.2f} Hz, expected: {expected_sampling_rate} Hz')
    print(f'active optical channels: {active_labels}')
    if peak_bpm is not None:
        print(f'peak estimate: {peak_bpm:.1f} BPM from {peaks.size} peaks')
    else:
        print('peak estimate: unavailable')
    if spectrum_bpm is not None:
        print(f'spectrum estimate: {spectrum_bpm:.1f} BPM')
    else:
        print('spectrum estimate: unavailable')
    print(f'wrote plot: {raw_plot}')
    print(f'wrote plot: {filtered_plot}')
    print(f'wrote plot: {pulse_plot}')
    print(f'wrote plot: {spectrum_plot}')


if __name__ == '__main__':
    main()
