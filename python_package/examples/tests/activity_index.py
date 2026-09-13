import time
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes


def demo():
    BoardShim.enable_dev_board_logger()

    # use synthetic board for demo
    params = BrainFlowInputParams()
    board_id = BoardIds.SYNTHETIC_BOARD.value
    board = BoardShim(board_id, params)
    board.prepare_session()
    board.start_stream()
    BoardShim.log_message(LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    sampling_rate = BoardShim.get_sampling_rate(board_id)
    accel_channels = BoardShim.get_accel_channels(board_id)
    print(f'Accel channels: {accel_channels}, Sampling rate: {sampling_rate}')

    # 1. Per-1-second epoch Activity Index (period = sampling_rate)
    ai_per_sec = DataFilter.get_activity_index(
        data[accel_channels[0]], data[accel_channels[1]], data[accel_channels[2]],
        sampling_rate, period=sampling_rate
    )
    print(f'Activity Index per 1-second epoch: {ai_per_sec}')

    # 2. Aggregate Activity Index over entire recording (default period = 0)
    ai_total = DataFilter.get_activity_index(
        data[accel_channels[0]], data[accel_channels[1]], data[accel_channels[2]],
        sampling_rate
    )
    print(f'Aggregate Activity Index over recording: {ai_total}')

    assert len(ai_per_sec) == len(data[0]) // sampling_rate
    assert len(ai_total) == 1
    assert np.isclose(np.sum(ai_per_sec), ai_total[0])


def test_regression():
    fs = 50

    # 1. Known numerical output and noise subtraction
    # Sine wave with amplitude 1.0 has variance = 0.5; flat signal has variance = 0.0
    t = np.linspace(0, 1.0, fs, endpoint=False)
    sig_x = np.sin(2 * np.pi * 2 * t)
    sig_y = np.cos(2 * np.pi * 2 * t)
    sig_z = np.zeros(fs)
    noise = 0.1

    var_x, var_y, var_z = float(np.var(sig_x)), float(np.var(sig_y)), float(np.var(sig_z))
    expected_ai = np.sqrt(max(0.0, ((var_x - noise) + (var_y - noise) + (var_z - 0.0)) / 3.0))

    ai = DataFilter.get_activity_index(
        sig_x, sig_y, sig_z, sampling_rate=fs, period=fs,
        noise_var_x=noise, noise_var_y=noise, noise_var_z=0.0
    )
    assert np.isclose(ai[0], expected_ai)

    # Noise subtraction clamping: if noise >= variance, AI must clamp to 0.0
    ai_clamped = DataFilter.get_activity_index(
        sig_x, sig_y, sig_z, sampling_rate=fs, period=fs,
        noise_var_x=10.0, noise_var_y=10.0, noise_var_z=10.0
    )
    assert np.isclose(ai_clamped[0], 0.0)

    # 2. Aggregation semantics: 2-second epoch equals sum of two 1-second AIs (Bai et al. 2016)
    sig2_x = np.concatenate([sig_x, 2.0 * sig_x])
    sig2_y = np.concatenate([sig_y, 2.0 * sig_y])
    sig2_z = np.zeros(2 * fs)

    ai_1s_1 = DataFilter.get_activity_index(sig_x, sig_y, sig_z, sampling_rate=fs, period=fs)[0]
    ai_1s_2 = DataFilter.get_activity_index(2.0 * sig_x, 2.0 * sig_y, sig_z, sampling_rate=fs, period=fs)[0]
    ai_2s = DataFilter.get_activity_index(sig2_x, sig2_y, sig2_z, sampling_rate=fs, period=2 * fs)[0]
    assert np.isclose(ai_2s, ai_1s_1 + ai_1s_2)

    # 3. Invalid arguments
    invalid_cases = [
        ([np.nan] * fs, [0.0] * fs, [0.0] * fs, fs, fs, 0.0),
        ([np.inf] * fs, [0.0] * fs, [0.0] * fs, fs, fs, 0.0),
        ([0.0] * fs, [0.0] * fs, [0.0] * fs, fs, fs, np.nan),
        ([0.0] * fs, [0.0] * (fs // 2), [0.0] * fs, fs, fs, 0.0),
        ([0.0] * fs, [0.0] * fs, [0.0] * fs, fs, fs - 1, 0.0),
        ([0.0] * fs, [0.0] * fs, [0.0] * fs, fs, 2 * fs, 0.0),
    ]
    for ax, ay, az, sr, per, n_var in invalid_cases:
        try:
            DataFilter.get_activity_index(
                np.array(ax, dtype=np.float64), np.array(ay, dtype=np.float64), np.array(az, dtype=np.float64),
                sampling_rate=sr, period=per, noise_var_x=n_var
            )
            assert False, f"Expected INVALID_ARGUMENTS_ERROR for inputs: sr={sr}, per={per}"
        except BrainFlowError as e:
            assert e.exit_code == BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value


def main():
    demo()
    test_regression()
    print('Activity index demo and regression tests passed successfully!')


if __name__ == '__main__':
    main()
