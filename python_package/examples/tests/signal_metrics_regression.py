"""PPG validity, rolling peaks, clipping, flatline and stable activity regressions."""

import ctypes
import unittest

import numpy as np

from brainflow.data_filter import DataHandlerDLL


DOUBLE = ctypes.POINTER(ctypes.c_double)
INT = ctypes.c_int
REAL = ctypes.c_double
INVALID = 13
BUFFER_ERROR = 9


def pointer(array):
    return array.ctypes.data_as(DOUBLE)


class SignalMetricsRegression(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        library = DataHandlerDLL.get_instance().lib

        def native(name, *arguments):
            return staticmethod(ctypes.CFUNCTYPE(INT, *arguments)((name, library)))

        cls.heart_rate = native(
            'get_heart_rate', DOUBLE, DOUBLE, INT, INT, INT, DOUBLE)
        cls.oxygen = native(
            'get_oxygen_level', DOUBLE, DOUBLE, INT, INT, REAL, REAL, REAL, DOUBLE)
        cls.peaks = native(
            'detect_peaks_z_score', DOUBLE, INT, INT, REAL, REAL, DOUBLE)
        cls.railed = native('get_railed_percentage', DOUBLE, INT, INT, DOUBLE)
        cls.clipping = native(
            'get_clipping_percentage', DOUBLE, INT, REAL, REAL, DOUBLE)
        cls.flatline = native('get_flatline_percentage', DOUBLE, INT, REAL, DOUBLE)
        cls.activity = native(
            'get_activity_index', DOUBLE, DOUBLE, DOUBLE, INT, INT, INT,
            REAL, REAL, REAL, DOUBLE)

    @staticmethod
    def ppg(frequency=1.23, sampling_rate=128, seconds=32):
        time = np.arange(sampling_rate * seconds) / sampling_rate
        pulse = np.sin(2 * np.pi * frequency * time + 0.23)
        return 1000.0 + 10.0 * pulse, 800.0 + 4.0 * pulse

    def test_heart_rate_rejects_absent_nonfinite_and_trend_only_pulses(self):
        output = np.array([321.0])
        for data in (np.zeros(4096), np.full(4096, 1000.), np.full(4096, np.nan),
                     np.full(4096, np.inf), 1000. + np.arange(4096.)):
            with self.subTest(value=data[0]):
                self.assertEqual(self.heart_rate(
                    pointer(data), pointer(data), data.size, 128, 1024, pointer(output)),
                    INVALID)
                self.assertEqual(output[0], 321.0)

    def test_heart_rate_interpolation_and_dc_scale_invariance(self):
        for frequency in (0.6, 1.23, 2.2, 3.7):
            ir, red = self.ppg(frequency)
            output = np.empty(1)
            self.assertEqual(self.heart_rate(
                pointer(ir), pointer(red), ir.size, 128, 1024, pointer(output)), 0)
            self.assertAlmostEqual(output[0], frequency * 60.0, delta=0.3)
            ir, red = ir * 1e100, red * 1e-100
            self.assertEqual(self.heart_rate(
                pointer(ir), pointer(red), ir.size, 128, 1024, pointer(output)), 0)
            self.assertAlmostEqual(output[0], frequency * 60.0, delta=0.3)

    def test_ppg_rejects_noise_and_disagreeing_channels(self):
        generator = np.random.default_rng(3)
        ir_noise = 1000. + generator.normal(size=4096)
        red_noise = 800. + generator.normal(size=4096)
        first, _ = self.ppg(1.0)
        _, second = self.ppg(2.0)
        output = np.array([321.0])
        for ir, red in ((ir_noise, red_noise), (first, second)):
            self.assertEqual(self.heart_rate(
                pointer(ir), pointer(red), ir.size, 128, 1024, pointer(output)), INVALID)
            self.assertEqual(self.oxygen(
                pointer(ir), pointer(red), ir.size, 128, 1., 2., 3., pointer(output)),
                INVALID)
        self.assertEqual(output[0], 321.0)

    def test_heart_rate_rejects_invalid_sampling_fft_and_short_input(self):
        ir, red = self.ppg()
        output = np.array([321.0])
        for sampling_rate, fft_size, length in ((0, 1024, 4096), (7, 1024, 4096),
                                                (128, 1023, 4096), (128, 512, 4096),
                                                (1000, 1024, 4096), (128, 8192, 4096)):
            self.assertEqual(self.heart_rate(
                pointer(ir), pointer(red), length, sampling_rate, fft_size, pointer(output)),
                INVALID)
        self.assertEqual(output[0], 321.0)

    def test_oxygen_uses_measured_ac_dc_ratio_over_supported_pulse_range(self):
        for frequency in (0.8, 1.23, 2.2, 3.2):
            for sampling_rate in (64, 128, 250):
                ir, red = self.ppg(frequency, sampling_rate)
                output = np.empty(1)
                self.assertEqual(self.oxygen(
                    pointer(ir), pointer(red), ir.size, sampling_rate,
                    1., 2., 3., pointer(output)), 0)
                # (red AC/red DC)/(IR AC/IR DC) = 0.5, with polynomial r*r+2*r+3.
                self.assertAlmostEqual(output[0], 4.25, delta=0.003)

    def test_oxygen_rejects_invalid_inputs_and_insufficient_guard_data(self):
        output = np.array([321.0])
        for data in (np.zeros(4096), np.full(4096, 1000.), np.full(4096, np.nan),
                     np.full(4096, np.inf), 1000. + np.arange(4096.)):
            self.assertEqual(self.oxygen(
                pointer(data), pointer(data), data.size, 128, 1., 2., 3., pointer(output)),
                INVALID)
        ir, red = self.ppg()
        for sampling_rate, coefficient in ((0, 1.), (8, 1.), (128, np.nan), (128, np.inf)):
            self.assertEqual(self.oxygen(
                pointer(ir), pointer(red), ir.size, sampling_rate,
                coefficient, 2., 3., pointer(output)), INVALID)
        self.assertEqual(self.oxygen(
            pointer(ir), pointer(red), 128, 128, 1., 2., 3., pointer(output)), BUFFER_ERROR)
        self.assertEqual(output[0], 321.0)

    @staticmethod
    def reference_peaks(data, lag, threshold, influence):
        filtered = data.copy()
        result = np.zeros_like(data)
        for i in range(lag, data.size):
            baseline = filtered[i - lag:i]
            if abs(data[i] - baseline.mean()) > threshold * baseline.std():
                result[i] = 1.0 if data[i] > baseline.mean() else -1.0
                filtered[i] = influence * data[i] + (1 - influence) * filtered[i - 1]
        return result

    def test_peak_detector_trailing_window_matches_reference(self):
        data = np.arange(10., dtype=np.float64)
        output = np.empty_like(data)
        self.assertEqual(self.peaks(
            pointer(data), data.size, 2, 3., 1., pointer(output)), 0)
        np.testing.assert_array_equal(output, np.zeros_like(data))
        generator = np.random.default_rng(193)
        data = generator.normal(size=5000)
        data[100:105] += 10.
        data[500] -= 20.
        for lag in (2, 17, 100):
            for influence in (0., 0.4, 1.):
                expected = self.reference_peaks(data, lag, 3., influence)
                output = np.empty_like(data)
                self.assertEqual(self.peaks(
                    pointer(data), data.size, lag, 3., influence, pointer(output)), 0)
                np.testing.assert_array_equal(output, expected)

    def test_peak_detector_rejects_nonfinite_and_invalid_parameters(self):
        data = np.arange(10.)
        output = np.full_like(data, 321.)
        for threshold, influence in ((np.nan, 1.), (np.inf, 1.), (3., np.nan),
                                      (3., np.inf), (3., 1.5), (3., -1.)):
            self.assertEqual(self.peaks(
                pointer(data), data.size, 2, threshold, influence, pointer(output)), INVALID)
        data[2] = np.nan
        self.assertEqual(self.peaks(
            pointer(data), data.size, 2, 3., 1., pointer(output)), INVALID)
        np.testing.assert_array_equal(output, np.full_like(data, 321.))

    def test_clipping_and_flatline_have_explicit_distinct_semantics(self):
        data = np.array([-2., -1., 0., 0., 0.05, 1., 2.])
        output = np.empty(1)
        self.assertEqual(self.clipping(
            pointer(data), data.size, -1., 1., pointer(output)), 0)
        self.assertAlmostEqual(output[0], 400. / 7)
        self.assertEqual(self.flatline(
            pointer(data), data.size, 0.1, pointer(output)), 0)
        self.assertAlmostEqual(output[0], 100. / 3)
        self.assertEqual(self.flatline(
            pointer(data), data.size, 0., pointer(output)), 0)
        self.assertAlmostEqual(output[0], 100. / 6)
        self.assertEqual(self.clipping(
            pointer(data), data.size, 1., -1., pointer(output)), INVALID)
        self.assertEqual(self.flatline(
            pointer(data), data.size, -1., pointer(output)), INVALID)
        data[0] = np.nan
        self.assertEqual(self.clipping(
            pointer(data), data.size, -1., 1., pointer(output)), INVALID)
        self.assertEqual(self.flatline(
            pointer(data), data.size, 0., pointer(output)), INVALID)

    def test_legacy_railed_metric_is_bounded_and_rejects_nonfinite_data(self):
        output = np.empty(1)
        data = np.array([0., 1e100, -1e100])
        self.assertEqual(self.railed(pointer(data), data.size, 24, pointer(output)), 0)
        self.assertEqual(output[0], 100.)
        data = np.full(128, np.nan)
        self.assertEqual(self.railed(pointer(data), data.size, 24, pointer(output)), INVALID)

    def test_activity_remains_finite_when_variance_would_overflow(self):
        output = np.empty(1)
        constant = np.full(128, 1e308)
        self.assertEqual(self.activity(
            pointer(constant), pointer(constant), pointer(constant), 128, 128, 128,
            0., 0., 0., pointer(output)), 0)
        self.assertEqual(output[0], 0.)
        data = np.tile(np.array([-1e200, 1e200]), 64)
        self.assertEqual(self.activity(
            pointer(data), pointer(data), pointer(data), 128, 128, 128,
            0., 0., 0., pointer(output)), 0)
        self.assertAlmostEqual(output[0] / 1e200, 1., places=12)


if __name__ == '__main__':
    unittest.main()
