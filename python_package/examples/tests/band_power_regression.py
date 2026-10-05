"""Numerical regressions for band integration and multichannel band powers.

Run against freshly built native libraries:
    python python_package/examples/tests/band_power_regression.py
"""

import unittest

import numpy as np

from brainflow.data_filter import DataFilter
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes


DEFAULT_BANDS = [(2.0, 4.0), (4.0, 8.0), (8.0, 13.0), (13.0, 30.0), (30.0, 45.0)]


def signal(sampling_rate=128, seconds=40, tones=((10.0, 1.0), (20.0, 2.0)), phase=0.37):
    time = np.arange(int(sampling_rate * seconds), dtype=np.float64) / sampling_rate
    return sum(amplitude * np.sin(2.0 * np.pi * frequency * time + phase)
               for frequency, amplitude in tones)


class BandPowerRegression(unittest.TestCase):
    def assert_invalid(self, function, *args, code=BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR):
        with self.assertRaises(BrainFlowError) as error:
            function(*args)
        self.assertEqual(error.exception.exit_code, code.value)

    def test_partial_bins_use_interpolated_endpoints(self):
        frequencies = np.arange(6, dtype=np.float64)
        amplitudes = 2.0 + frequencies
        lower, upper = 0.25, 3.75
        # Integral of the linear density 2 + f, independent of the bin boundaries.
        expected = 2.0 * (upper - lower) + (upper ** 2 - lower ** 2) / 2.0
        actual = DataFilter.get_band_power((amplitudes, frequencies), lower, upper)
        self.assertAlmostEqual(actual, expected, places=12)

    def test_adjacent_bands_partition_power_without_double_counting(self):
        frequencies = np.arange(6, dtype=np.float64)
        amplitudes = np.array([1.0, 2.0, 7.0, 3.0, 4.0, 2.0])
        psd = amplitudes, frequencies
        for boundary in (2.0, 2.25):
            with self.subTest(boundary=boundary):
                left = DataFilter.get_band_power(psd, 0.0, boundary)
                right = DataFilter.get_band_power(psd, boundary, 5.0)
                whole = DataFilter.get_band_power(psd, 0.0, 5.0)
                self.assertAlmostEqual(left + right, whole, places=12)

    def test_invalid_psd_and_integration_bounds(self):
        frequencies = np.arange(6, dtype=np.float64)
        amplitudes = np.ones(6, dtype=np.float64)
        for lower, upper in ((-1.0, 2.0), (1.0, 6.0), (3.0, 2.0), (2.0, 2.0),
                             (np.nan, 2.0), (1.0, np.inf)):
            with self.subTest(lower=lower, upper=upper):
                self.assert_invalid(DataFilter.get_band_power,
                                    (amplitudes, frequencies), lower, upper)
        for bad in (np.nan, np.inf):
            with self.subTest(value=bad):
                bad_amplitudes = amplitudes.copy()
                bad_amplitudes[2] = bad
                self.assert_invalid(DataFilter.get_band_power,
                                    (bad_amplitudes, frequencies), 0.0, 5.0)
                bad_frequencies = frequencies.copy()
                bad_frequencies[2] = bad
                self.assert_invalid(DataFilter.get_band_power,
                                    (amplitudes, bad_frequencies), 0.0, 5.0)

    def test_known_tone_powers_and_channel_variation(self):
        waveform = signal(seconds=6)
        data = np.vstack((waveform, 2.0 * waveform))
        average, variation = DataFilter.get_custom_band_powers(
            data, [(8.0, 13.0), (17.0, 23.0)], [0, 1], 128, False)
        # Sine powers are proportional to squared amplitudes: 1 and 4.
        np.testing.assert_allclose(average, [0.2, 0.8], atol=1e-8, rtol=0.0)
        # Across channels, powers also have ratio 1:4; population std / mean = 0.6.
        np.testing.assert_allclose(variation, [0.6, 0.6], atol=1e-12, rtol=0.0)

    def test_common_amplitude_scaling_preserves_normalized_outputs(self):
        waveform = signal()
        data = np.vstack((waveform, 2.0 * waveform))
        for filtered in (False, True):
            with self.subTest(filtered=filtered):
                baseline = DataFilter.get_avg_band_powers(data, [0, 1], 128, filtered)
                scaled = DataFilter.get_avg_band_powers(7.0 * data, [0, 1], 128, filtered)
                np.testing.assert_allclose(scaled[0], baseline[0], atol=1e-10, rtol=1e-8)
                # Tiny leakage-only bands can be less stable than the measured tones.
                np.testing.assert_allclose(scaled[1][2:4], baseline[1][2:4],
                                           atol=1e-10, rtol=1e-8)

    def test_default_bands_use_custom_implementation(self):
        data = np.vstack((signal(), signal(tones=((6.0, 1.0), (35.0, 1.0)))))
        for filtered in (False, True):
            with self.subTest(filtered=filtered):
                average = DataFilter.get_avg_band_powers(data, [0, 1], 128, filtered)
                custom = DataFilter.get_custom_band_powers(
                    data, DEFAULT_BANDS, [0, 1], 128, filtered)
                np.testing.assert_allclose(average, custom, atol=0.0, rtol=0.0)

    def test_zero_input_returns_finite_zeros(self):
        data = np.zeros((2, 128 * 40), dtype=np.float64)
        for filtered in (False, True):
            with self.subTest(filtered=filtered):
                average, variation = DataFilter.get_avg_band_powers(data, [0, 1], 128, filtered)
                np.testing.assert_array_equal(average, np.zeros(5))
                np.testing.assert_array_equal(variation, np.zeros(5))

    def test_constant_nonzero_channels_return_exact_zeros_after_filtering(self):
        data = np.vstack((np.full(128 * 40, 0.1), np.full(128 * 40, 1e6)))
        average, variation = DataFilter.get_avg_band_powers(data, [0, 1], 128, True)
        np.testing.assert_array_equal(average, np.zeros(5))
        np.testing.assert_array_equal(variation, np.zeros(5))

    def test_large_finite_signal_does_not_overflow_channel_variation(self):
        waveform = 1e100 * signal(seconds=6)
        data = np.vstack((waveform, 2.0 * waveform))
        average, variation = DataFilter.get_custom_band_powers(
            data, [(8.0, 13.0), (17.0, 23.0)], [0, 1], 128, False)
        # Powers fit in a double, but squaring their unscaled deviations would overflow.
        np.testing.assert_allclose(average, [0.2, 0.8], atol=1e-8, rtol=0.0)
        np.testing.assert_allclose(variation, [0.6, 0.6], atol=1e-12, rtol=0.0)

    def test_nonfinite_samples_are_rejected(self):
        for filtered in (False, True):
            for bad in (np.nan, np.inf, -np.inf):
                with self.subTest(filtered=filtered, value=bad):
                    data = np.vstack((signal(), signal()))
                    data[1, data.shape[1] // 2] = bad
                    self.assert_invalid(DataFilter.get_avg_band_powers,
                                        data, [0, 1], 128, filtered)

    def test_invalid_custom_band_bounds_are_rejected(self):
        data = signal()[np.newaxis, :]
        for filtered in (False, True):
            for band in ((-1.0, 4.0), (8.0, 8.0), (12.0, 8.0), (1.0, 65.0),
                         (np.nan, 4.0), (1.0, np.inf)):
                with self.subTest(filtered=filtered, band=band):
                    self.assert_invalid(DataFilter.get_custom_band_powers,
                                        data, [band], [0], 128, filtered)

    def test_short_input_does_not_silently_lower_frequency_resolution(self):
        sampling_rate = 128
        nfft = 2 * DataFilter.get_nearest_power_of_two(sampling_rate)
        # Unfiltered input must accommodate the full FFT. Filtering needs extra
        # samples on both ends, even when an FFT alone would fit.
        for filtered, length in ((False, nfft - 1), (True, nfft + 1)):
            with self.subTest(filtered=filtered, length=length):
                data = np.ones((1, length), dtype=np.float64)
                self.assert_invalid(DataFilter.get_avg_band_powers,
                                    data, [0], sampling_rate, filtered,
                                    code=BrainFlowExitCodes.INVALID_BUFFER_SIZE_ERROR)

        data = signal(sampling_rate, seconds=nfft / sampling_rate)[np.newaxis, :]
        average, _ = DataFilter.get_avg_band_powers(data, [0], sampling_rate, False)
        self.assertAlmostEqual(float(np.sum(average)), 1.0, places=12)

    def test_custom_filter_preserves_requested_low_and_high_frequencies(self):
        data = signal(256, tones=((1.0, 1.0), (10.0, 1.0), (75.0, 1.0)))[np.newaxis, :]
        average, variation = DataFilter.get_custom_band_powers(
            data, [(0.25, 2.0), (8.0, 12.0), (65.0, 100.0)], [0], 256, True)
        # A fixed 2-45 Hz passband suppresses the outer tones almost entirely.
        self.assertTrue(np.all(average > 0.2), average)
        self.assertAlmostEqual(float(np.sum(average)), 1.0, places=12)
        np.testing.assert_array_equal(variation, np.zeros(3))

    def test_filters_work_when_mains_notches_are_above_nyquist(self):
        data = signal(100, tones=((10.0, 1.0), (20.0, 1.0)))[np.newaxis, :]
        average, variation = DataFilter.get_custom_band_powers(
            data, [(8.0, 13.0), (13.0, 30.0)], [0], 100, True)
        self.assertTrue(np.all(np.isfinite(average)), average)
        self.assertTrue(np.all(average > 0.25), average)
        self.assertAlmostEqual(float(np.sum(average)), 1.0, places=12)
        np.testing.assert_array_equal(variation, np.zeros(2))

    def test_custom_range_can_start_at_zero(self):
        data = signal(128, tones=((1.0, 1.0), (10.0, 1.0)))[np.newaxis, :]
        average, variation = DataFilter.get_custom_band_powers(
            data, [(0.0, 3.0), (8.0, 16.0)], [0], 128, True)
        self.assertTrue(np.all(average > 0.35), average)
        self.assertAlmostEqual(float(np.sum(average)), 1.0, places=12)
        np.testing.assert_array_equal(variation, np.zeros(2))

    def test_filtering_removes_dc_offsets(self):
        data = signal()[np.newaxis, :]
        reference, _ = DataFilter.get_avg_band_powers(data, [0], 128, True)
        shifted, _ = DataFilter.get_avg_band_powers(data + 1e6, [0], 128, True)
        np.testing.assert_allclose(shifted, reference, atol=1e-8, rtol=0.0)

    def test_short_recording_with_endpoint_transients_matches_long_reference(self):
        # Eight seconds is close to the default filter's minimum at 128 Hz,
        # making retained samples near the exclusion margins matter to the PSD.
        # The 3 Hz tone also probes the lower edge of the analysis passband.
        tones = ((3.0, 1.0), (10.0, 1.0), (20.0, 2.0))
        for phase in (0.0, 0.7, 2.0):
            with self.subTest(phase=phase):
                long_data = signal(tones=tones, phase=phase)[np.newaxis, :]
                reference, _ = DataFilter.get_avg_band_powers(long_data, [0], 128, True)
                short_data = signal(seconds=8, tones=tones, phase=phase)[np.newaxis, :]
                short_data[0, 0] += 1000.0
                short_data[0, -1] -= 250.0
                actual, _ = DataFilter.get_avg_band_powers(short_data, [0], 128, True)
                np.testing.assert_allclose(actual, reference, atol=2e-5, rtol=0.0)

    def test_input_is_unchanged(self):
        data = np.vstack((signal(), signal(tones=((6.0, 1.0),))))
        original = data.copy()
        for filtered in (False, True):
            with self.subTest(filtered=filtered):
                DataFilter.get_avg_band_powers(data, [0, 1], 128, filtered)
                np.testing.assert_array_equal(data, original)


if __name__ == '__main__':
    unittest.main()
