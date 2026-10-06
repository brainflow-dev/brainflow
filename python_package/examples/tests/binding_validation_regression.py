"""Binding shape validation and checked inverse-wavelet regression tests."""
import unittest

import numpy as np

from brainflow.data_filter import DataFilter, DetrendOperations, FilterTypes, StreamingFilter, WaveletTypes, WindowOperations
from brainflow.exit_codes import BrainFlowError


class BindingValidationRegression(unittest.TestCase):
    def test_psd_arrays_must_match(self):
        with self.assertRaises(BrainFlowError):
            DataFilter.get_band_power((np.ones(5), np.arange(4, dtype=float)), 1.0, 2.0)

    def test_psd_views_must_be_contiguous(self):
        with self.assertRaises(BrainFlowError):
            DataFilter.get_band_power((np.ones(10)[::2], np.arange(5, dtype=float)), 1.0, 2.0)

    def test_csp_labels_and_layout(self):
        data = np.ones((4, 2, 20))
        for labels in (np.zeros(3), np.zeros(8)[::2]):
            with self.subTest(labels=labels), self.assertRaises(BrainFlowError):
                DataFilter.get_csp(data, labels)

    def test_inplace_filters_reject_readonly_data(self):
        data = np.ones(256)
        data.flags.writeable = False
        operations = [
            lambda: DataFilter.perform_lowpass(data, 256, 30, 2, FilterTypes.BUTTERWORTH, 0),
            lambda: DataFilter.perform_highpass(data, 256, 2, 2, FilterTypes.BUTTERWORTH, 0),
            lambda: DataFilter.perform_bandpass(data, 256, 2, 30, 2, FilterTypes.BUTTERWORTH, 0),
            lambda: DataFilter.perform_bandstop(data, 256, 48, 52, 2, FilterTypes.BUTTERWORTH, 0),
            lambda: DataFilter.remove_environmental_noise(data, 256, 0),
            lambda: DataFilter.perform_rolling_filter(data, 4, 0),
            lambda: DataFilter.perform_wavelet_denoising(data, WaveletTypes.DB3, 2),
            lambda: DataFilter.detrend(data, 1),
        ]
        for operation in operations:
            with self.subTest(operation=operation), self.assertRaises(BrainFlowError):
                operation()
        np.testing.assert_array_equal(data, 1)

    def test_inverse_wavelet_rejects_short_and_inconsistent_metadata(self):
        coefficients = np.zeros(72)
        lengths = np.array([19, 19, 34], dtype=np.int32)
        cases = [(coefficients[:1], lengths), (coefficients, lengths[:1]),
                 (coefficients, np.array([-1, 39, 34], dtype=np.int32)),
                 (coefficients, lengths.astype(np.int64))]
        for output in cases:
            with self.subTest(output=output), self.assertRaises(BrainFlowError):
                DataFilter.perform_inverse_wavelet_transform(output, 64, WaveletTypes.DB3, 2)

    def test_inverse_wavelet_checked_native_configuration(self):
        data = np.random.default_rng(7).normal(size=64)
        transformed = DataFilter.perform_wavelet_transform(data, WaveletTypes.DB3, 2)
        restored = DataFilter.perform_inverse_wavelet_transform(transformed, 64, WaveletTypes.DB3, 2)
        np.testing.assert_allclose(restored, data, atol=1e-10)
        with self.assertRaises(BrainFlowError):
            DataFilter.perform_inverse_wavelet_transform(transformed, 32, WaveletTypes.DB3, 2)

    def test_welch_odd_recording(self):
        data = np.sin(2 * np.pi * 10 * np.arange(1025) / 256)
        amplitudes, frequencies = DataFilter.get_psd_welch(data, 256, 128, 256, WindowOperations.HANNING)
        self.assertEqual(amplitudes.size, 129)
        self.assertEqual(frequencies.size, 129)
        self.assertEqual(frequencies[-1], 128)

    def test_band_power_metadata_matches_required_input(self):
        options = dict(nfft=256, low_cutoff=2.0, high_cutoff=40.0, mains=0)
        info = DataFilter.get_band_power_info(256, **options)
        self.assertEqual(info['nfft'], 256)
        self.assertEqual(info['frequency_resolution'], 1.0)
        self.assertEqual(info['minimum_samples'], 256 + 2 * info['edge_samples'])
        data = np.random.default_rng(42).normal(size=(2, info['minimum_samples']))
        bands = [(2, 4), (8, 12)]
        powers, _ = DataFilter.get_custom_band_powers_with_options(data, bands, [0, 1], 256, **options)
        self.assertTrue(np.isfinite(powers).all())
        ready = DataFilter.get_band_power_info(256, data_len=data.shape[1], **options)
        self.assertEqual(ready['usable_stop'] - ready['usable_start'], 256)
        with self.assertRaises(BrainFlowError):
            DataFilter.get_custom_band_powers_with_options(data[:, :-1].copy(), bands, [0, 1], 256, **options)

    def test_adding_bands_does_not_change_preprocessing(self):
        time = np.arange(4096) / 125
        data = (np.sin(2 * np.pi * 2 * time) + np.sin(2 * np.pi * 10 * time))[None, :]
        first, _ = DataFilter.get_custom_band_powers(data, [(2, 4), (8, 12)], [0], 125, True)
        extended, _ = DataFilter.get_custom_band_powers(
            data, [(0, 1), (2, 4), (8, 12), (20, 30)], [0], 125, True)
        self.assertAlmostEqual(first[0] / first[1], extended[1] / extended[2], places=12)

    def test_band_power_channel_selection_and_resolution(self):
        data = np.random.default_rng(3).normal(size=(2, 2048))
        for channels in ([-1], [2], [0.5]):
            with self.subTest(channels=channels), self.assertRaises(BrainFlowError):
                DataFilter.get_avg_band_powers(data, channels, 256, False)
        selected = [1, 0, 1]
        bands = [(2, 8), (8, 30)]
        actual = DataFilter.get_custom_band_powers(data, bands, selected, 256, False)
        expected = DataFilter.get_custom_band_powers(data[selected].copy(), bands, [0, 1, 2], 256, False)
        for left, right in zip(actual, expected):
            np.testing.assert_allclose(left, right)
        info = DataFilter.get_band_power_info(256, False, nfft=1024)
        self.assertEqual(info['frequency_resolution'], 0.25)
        self.assertEqual(info['edge_samples'], 0)
        narrow, _ = DataFilter.get_custom_band_powers_with_options(
            data, [(8.25, 8.75), (10.25, 10.75)], [0, 1], 256, False, nfft=1024,
            detrend_operation=DetrendOperations.NO_DETREND)
        self.assertAlmostEqual(narrow.sum(), 1.0)

    def test_ica_reproducible_controls_and_reconstruction(self):
        source = np.random.default_rng(11).laplace(size=(2, 2000))
        data = np.ascontiguousarray(np.array([[1., 2.], [3., 4.], [.5, 1.5]]) @ source)
        first = DataFilter.perform_ica(data, 2, seed=7)
        second = DataFilter.perform_ica(data, 2, seed=7)
        for left, right in zip(first, second):
            np.testing.assert_array_equal(left, right)
        np.testing.assert_allclose(first[2] @ first[3], data - data.mean(axis=1, keepdims=True), atol=1e-10)
        with self.assertRaises(BrainFlowError):
            DataFilter.perform_ica(data, 2, max_iterations=1, seed=7)

    def test_explicit_clipping_and_flatline_metrics(self):
        self.assertEqual(DataFilter.get_clipping_percentage(np.array([-2., -1., 0., 1., 2.]), -1, 1), 80)
        self.assertEqual(DataFilter.get_flatline_percentage(np.array([1., 1., 2., 2., 2.])), 75)

    def test_wavelet_dimensions_rejected_before_allocation(self):
        for level in (-1, 0, 101, 2147483647):
            with self.subTest(level=level), self.assertRaises(BrainFlowError):
                DataFilter.perform_wavelet_transform(np.ones(64), WaveletTypes.DB3, level)
        with self.assertRaises(BrainFlowError):
            DataFilter.perform_inverse_wavelet_transform(
                (np.zeros(72), np.array([19, 19, 34], dtype=np.int32)), 2147483647, WaveletTypes.DB3, 2)

    def test_streaming_chunks_and_reset_match_causal_filter(self):
        data = np.random.default_rng(3).normal(size=1024)
        expected = data.copy()
        DataFilter.perform_lowpass(expected, 256, 30., 4, FilterTypes.BUTTERWORTH, 1.)
        chunks = [chunk.copy() for chunk in np.split(data, [1, 33, 241, 888])]
        with StreamingFilter(0, 256, low_cutoff=30.) as stream:
            for chunk in chunks:
                stream.process(chunk)
            np.testing.assert_allclose(np.concatenate(chunks), expected, atol=1e-12)
            stream.reset()
            again = data.copy()
            stream.process(again)
            np.testing.assert_array_equal(again, np.concatenate(chunks))
        stream.close()
        with self.assertRaises(BrainFlowError):
            stream.process(data.copy())

    def test_invalid_streaming_chunk_preserves_state_and_data(self):
        data = np.random.default_rng(7).normal(size=128)
        with StreamingFilter(0, 256, low_cutoff=30.) as stream, StreamingFilter(0, 256, low_cutoff=30.) as reference:
            warmup = data.copy()
            stream.process(warmup)
            reference.process(data.copy())
            bad = np.array([1., np.nan, 3.])
            with self.assertRaises(BrainFlowError):
                stream.process(bad)
            np.testing.assert_array_equal(bad, [1., np.nan, 3.])
            actual, expected = data.copy(), data.copy()
            stream.process(actual)
            reference.process(expected)
            np.testing.assert_array_equal(actual, expected)

    def test_decimation_preserves_passband_and_suppresses_alias(self):
        time = np.arange(4097) / 256
        low = np.sin(2 * np.pi * 8 * time)
        high = np.sin(2 * np.pi * 80 * time)
        decimated_low = DataFilter.perform_decimation(low, 4)
        decimated_high = DataFilter.perform_decimation(high, 4)
        self.assertEqual(decimated_low.size, 1024)
        np.testing.assert_allclose(decimated_low[20:-20], low[:4096:4][20:-20], atol=0.01)
        self.assertLess(np.sqrt(np.mean(decimated_high[20:-20] ** 2)), 0.01)
        np.testing.assert_array_equal(DataFilter.perform_decimation(low, 1), low)

    def test_public_filter_margin_query(self):
        margin = DataFilter.get_filter_settling_samples(0, 256, low_cutoff=30.)
        self.assertGreater(margin, 0)
        with self.assertRaises(BrainFlowError):
            DataFilter.get_filter_settling_samples(0, 256, low_cutoff=200.)


if __name__ == '__main__':
    unittest.main()
