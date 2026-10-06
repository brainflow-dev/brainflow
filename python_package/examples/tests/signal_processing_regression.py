"""Numerical and input-boundary regressions for the native signal-processing core.

Run against freshly built libraries with this checkout's Python package on PYTHONPATH.
"""

import tempfile
import unittest
from pathlib import Path

import numpy as np
from scipy import signal

from brainflow.data_filter import DataFilter, DataHandlerDLL
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes


class SignalProcessingRegression(unittest.TestCase):
    def assert_invalid(self, function, *args):
        with self.assertRaises(BrainFlowError) as error:
            function(*args)
        self.assertEqual(error.exception.exit_code, BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)

    def test_periodogram_matches_scipy_for_every_window(self):
        data = np.random.default_rng(21).normal(size=1000)
        for window in range(4):
            with self.subTest(window=window):
                weights = DataFilter.get_window(window, len(data))
                frequencies, expected = signal.periodogram(data, fs=250, window=weights, detrend=False)
                actual, actual_frequencies = DataFilter.get_psd(data, 250, window)
                np.testing.assert_allclose(actual_frequencies, frequencies, atol=1e-13)
                np.testing.assert_allclose(actual, expected, rtol=2e-12, atol=1e-15)

    def test_welch_matches_scipy_with_odd_recording_length(self):
        data = np.random.default_rng(22).normal(size=2049)
        for window in range(4):
            with self.subTest(window=window):
                weights = DataFilter.get_window(window, 256)
                frequencies, expected = signal.welch(data, fs=256, window=weights,
                    nperseg=256, noverlap=97, detrend=False)
                actual, actual_frequencies = DataFilter.get_psd_welch(data, 256, 97, 256, window)
                self.assertEqual(len(actual), 129)
                np.testing.assert_allclose(actual_frequencies, frequencies, atol=1e-13)
                np.testing.assert_allclose(actual, expected, rtol=2e-12, atol=1e-15)

    def test_psd_scaling_avoids_integer_overflow(self):
        data = np.cos(2 * np.pi * 31 * np.arange(4096) / 4096)
        frequencies, expected = signal.periodogram(data, fs=1_000_000, detrend=False)
        actual, actual_frequencies = DataFilter.get_psd(data, 1_000_000, 0)
        np.testing.assert_allclose(actual_frequencies, frequencies)
        np.testing.assert_allclose(actual, expected, rtol=1e-12, atol=1e-20)

    def test_fft_and_ifft_match_numpy_and_preserve_input(self):
        data = np.random.default_rng(23).normal(size=1000)
        original = data.copy()
        for window in range(4):
            with self.subTest(window=window):
                weights = DataFilter.get_window(window, len(data))
                actual = DataFilter.perform_fft(data, window)
                np.testing.assert_allclose(actual, np.fft.rfft(data * weights), rtol=1e-12, atol=1e-12)
                np.testing.assert_allclose(DataFilter.perform_ifft(actual), data * weights, atol=1e-12)
        np.testing.assert_array_equal(data, original)

    def test_spectral_nonfinite_inputs_are_rejected(self):
        for value in (np.nan, np.inf, -np.inf):
            data = np.ones(256)
            data[100] = value
            self.assert_invalid(DataFilter.perform_fft, data, 0)
            self.assert_invalid(DataFilter.get_psd, data, 256, 0)
            self.assert_invalid(DataFilter.get_psd_welch, data, 64, 32, 256, 0)
            spectrum = np.ones(129, dtype=np.complex128)
            spectrum[20] = complex(value, 0)
            self.assert_invalid(DataFilter.perform_ifft, spectrum)

    def test_long_linear_detrend_matches_scipy(self):
        data = 3.0 + 0.02 * np.arange(200_000, dtype=np.float64)
        data += 0.5 * np.sin(np.arange(len(data)) * 0.11)
        expected = signal.detrend(data)
        DataFilter.detrend(data, 2)
        np.testing.assert_allclose(data, expected, atol=5e-11, rtol=0)

    def test_singleton_and_large_magnitude_statistics(self):
        for operation in (1, 2):
            data = np.array([1e308])
            DataFilter.detrend(data, operation)
            np.testing.assert_array_equal(data, [0])
        data = np.array([1e308, -1e308, 1e308, -1e308])
        self.assertAlmostEqual(DataFilter.calc_stddev(data) / 1e308, 1.0, places=14)
        constant = np.full(20, 1e308)
        DataFilter.detrend(constant, 1)
        np.testing.assert_array_equal(constant, np.zeros(20))

    def test_causal_butterworth_matches_scipy(self):
        original = np.random.default_rng(24).normal(size=2048)
        methods = ((DataFilter.perform_lowpass, 'lowpass', (30.0,)),
                   (DataFilter.perform_highpass, 'highpass', (5.0,)),
                   (DataFilter.perform_bandpass, 'bandpass', (5.0, 30.0)),
                   (DataFilter.perform_bandstop, 'bandstop', (48.0, 52.0)))
        for method, kind, cutoffs in methods:
            with self.subTest(kind=kind):
                expected = signal.sosfilt(signal.butter(4, cutoffs[0] if len(cutoffs) == 1 else cutoffs,
                    btype=kind, fs=256, output='sos'), original)
                actual = original.copy()
                method(actual, 256, *cutoffs, 4, 0, 0.0)
                np.testing.assert_allclose(actual, expected, atol=2e-11, rtol=1e-10)

    def test_zero_phase_filters_do_not_create_constant_edge_transients(self):
        for method, cutoffs, expected in ((DataFilter.perform_lowpass, (30.0,), 17.0),
                (DataFilter.perform_highpass, (5.0,), 0.0),
                (DataFilter.perform_bandpass, (5.0, 30.0), 0.0),
                (DataFilter.perform_bandstop, (48.0, 52.0), 17.0)):
            with self.subTest(method=method.__name__):
                actual = np.full(2048, 17.0)
                method(actual, 256, *cutoffs, 4, 3, 0.0)
                np.testing.assert_allclose(actual, expected, atol=1e-9, rtol=0)

    def test_both_mains_notches_are_applied_in_sequence(self):
        original = np.random.default_rng(25).normal(size=4096)
        combined = original.copy()
        sequential = original.copy()
        DataFilter.remove_environmental_noise(combined, 256, 2)
        DataFilter.perform_bandstop(sequential, 256, 48.0, 52.0, 4, 3, 0.0)
        DataFilter.perform_bandstop(sequential, 256, 58.0, 62.0, 4, 3, 0.0)
        np.testing.assert_array_equal(combined, sequential)

    def test_invalid_filter_design_preserves_input(self):
        original = np.arange(256, dtype=np.float64)
        invalid_calls = ((DataFilter.perform_lowpass, (256, 128.0, 4, 0, 0.0)),
                         (DataFilter.perform_highpass, (256, 0.0, 4, 0, 0.0)),
                         (DataFilter.perform_bandpass, (256, 30.0, 5.0, 4, 0, 0.0)),
                         (DataFilter.perform_bandstop, (256, 48.0, np.nan, 4, 0, 0.0)),
                         (DataFilter.perform_lowpass, (256, 30.0, 9, 0, 0.0)),
                         (DataFilter.perform_lowpass, (256, 30.0, 4, 1, 0.0)),
                         (DataFilter.remove_environmental_noise, (100, 2)))
        for method, args in invalid_calls:
            with self.subTest(method=method.__name__, args=args):
                actual = original.copy()
                self.assert_invalid(method, actual, *args)
                np.testing.assert_array_equal(actual, original)

    def test_rolling_mean_and_median_use_partial_warmup(self):
        original = np.array([7., 2., 5., 1., 1., 8., -3., 4., 4.])
        for period in (1, 2, 4, 20):
            for operation, aggregate in ((0, np.mean), (1, np.median)):
                with self.subTest(period=period, operation=operation):
                    expected = [aggregate(original[max(0, i-period+1):i+1]) for i in range(len(original))]
                    actual = original.copy()
                    DataFilter.perform_rolling_filter(actual, period, operation)
                    np.testing.assert_allclose(actual, expected, atol=1e-14, rtol=0)

    def test_huge_constant_zero_phase_filter_does_not_overflow_padding(self):
        data = np.full(128, 1e308)
        DataFilter.perform_lowpass(data, 256, 20.0, 4, 3, 0.0)
        np.testing.assert_allclose(data / 1e308, 1.0, atol=2e-14, rtol=0)

    def test_downsampling_without_a_complete_block_does_not_allocate_the_period(self):
        data = np.arange(8, dtype=np.float64)
        output = np.array([99.0])
        result = DataHandlerDLL.get_instance().perform_downsampling(data, len(data), 2147483647, 1, output)
        self.assertEqual(result, 0)
        np.testing.assert_array_equal(output, [99.0])

    def test_nearest_power_of_two_checks_upper_overflow(self):
        self.assertEqual(DataFilter.get_nearest_power_of_two(3), 4)
        self.assertEqual(DataFilter.get_nearest_power_of_two(2**30), 2**30)
        self.assertEqual(DataFilter.get_nearest_power_of_two(3 * 2**29 - 1), 2**30)
        for value in (0, -1, 3 * 2**29, 2**31 - 1):
            self.assert_invalid(DataFilter.get_nearest_power_of_two, value)

    def test_file_round_trip_preserves_full_precision_and_wide_rows(self):
        data = np.random.default_rng(26).normal(size=(3000, 3))
        with tempfile.TemporaryDirectory() as directory:
            path = str(Path(directory) / 'wide.tsv')
            DataFilter.write_file(data, path, 'w')
            np.testing.assert_array_equal(DataFilter.read_file(path), data)
            file_path = Path(path)
            file_path.write_bytes(file_path.read_bytes().rstrip(b'\r\n'))
            np.testing.assert_array_equal(DataFilter.read_file(path), data)

    def test_file_parser_rejects_partial_tokens_and_ragged_rows(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'invalid.tsv'
            for content in ('1x\t2\n', '1\t2\n3\n', '1\t2\t\n', '1\tNaN\n'):
                with self.subTest(content=content):
                    path.write_text(content)
                    self.assert_invalid(DataFilter.read_file, str(path))

    def test_native_file_read_enforces_actual_capacity_without_writing(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'data.tsv'
            path.write_text('1\t2\n3\t4')
            output = np.full(3, 99.0)
            rows = np.zeros(1, dtype=np.int32)
            cols = np.zeros(1, dtype=np.int32)
            result = DataHandlerDLL.get_instance().read_file(output, rows, cols, str(path).encode(), len(output))
            self.assertEqual(result, BrainFlowExitCodes.INVALID_BUFFER_SIZE_ERROR.value)
            np.testing.assert_array_equal(output, np.full(3, 99.0))


if __name__ == '__main__':
    unittest.main()
