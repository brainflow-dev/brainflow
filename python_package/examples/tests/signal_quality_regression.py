"""Quality rejection, diagnostics and legacy-DSP parity against freshly built DataHandler."""
import ctypes
from concurrent.futures import ThreadPoolExecutor
import json
import unittest
from unittest.mock import patch

import numpy as np

from brainflow.data_filter import DataFilter, DataHandlerDLL, SignalQualityOptions, SignalQualityFlags
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes
from brainflow.signal_quality import _NativeChannelQuality
from quality_band_powers import QualityTransitionLogger


class SignalQualityRegression(unittest.TestCase):
    def setUp(self):
        self.rate = 128
        t = np.arange(12 * self.rate) / self.rate
        self.data = np.array([np.sin(2*np.pi*10*t + .13),
                              2*np.sin(2*np.pi*20*t + .21),
                              3*np.sin(2*np.pi*6*t + .31)])
        self.policy = SignalQualityOptions(clipping_bounds=(-20, 20), min_channels=2)

    def bands(self, data=None, policy=None, channels=None, filtered=False):
        return DataFilter.get_avg_band_powers_with_quality(
            self.data if data is None else data, [0, 1, 2] if channels is None else channels,
            self.rate, filtered, self.policy if policy is None else policy)

    def invalid(self, call, code=BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR):
        with self.assertRaises(BrainFlowError) as error:
            call()
        self.assertEqual(error.exception.exit_code, code.value)

    def test_clean_data_is_identical_to_legacy_with_and_without_filtering(self):
        before = self.data.copy()
        for filtered in (False, True):
            result = self.bands(filtered=filtered)
            legacy = DataFilter.get_avg_band_powers(self.data, [0, 1, 2], self.rate, filtered)
            self.assertTrue(result['valid'])
            self.assertEqual(result['used_channels'], [0, 1, 2])
            np.testing.assert_array_equal(result['avg_band_powers'], legacy[0])
            np.testing.assert_array_equal(result['stddev_band_powers'], legacy[1])
        np.testing.assert_array_equal(self.data, before)

    def test_clipped_channel_is_excluded_before_filtering_and_normalization(self):
        self.data[1, 100:200] = 1e6
        before = self.data.copy()
        for filtered in (False, True):
            result = self.bands(filtered=filtered, channels=[2, 1, 0])
            expected = DataFilter.get_avg_band_powers(self.data, [2, 0], self.rate, filtered)
            self.assertEqual(result['used_channels'], [2, 0])
            self.assertEqual(result['rejected_channels'], [1])
            self.assertIn('clipped', result['quality'][1]['reasons'])
            np.testing.assert_array_equal(result['avg_band_powers'], expected[0])
            np.testing.assert_array_equal(result['stddev_band_powers'], expected[1])
        np.testing.assert_array_equal(self.data, before)

    def test_nonfinite_rejects_only_affected_channel_and_keeps_legacy_error(self):
        for value in (np.nan, np.inf, -np.inf):
            self.data[1, 100] = value
            result = self.bands()
            self.assertTrue(result['valid'])
            self.assertEqual(result['rejected_channels'], [1])
            self.assertEqual(result['quality'][1]['flags'], int(SignalQualityFlags.NONFINITE))
            self.assertAlmostEqual(result['quality'][1]['nonfinite_percentage'], 100/self.data.shape[1])
            self.invalid(lambda: DataFilter.get_avg_band_powers(self.data, [0, 1, 2], self.rate, False))

    def test_all_rejected_returns_diagnostics_and_no_powers(self):
        result = self.bands(np.zeros_like(self.data))
        self.assertFalse(result['valid'])
        self.assertEqual(result['reason'], 'insufficient_channels')
        self.assertIsNone(result['avg_band_powers'])
        self.assertIsNone(result['stddev_band_powers'])
        self.assertEqual(result['rejected_channels'], [0, 1, 2])
        json.dumps(result, allow_nan=False)

    def test_minimum_channels_is_enforced(self):
        self.data[1] = 0
        result = self.bands(policy=SignalQualityOptions(min_channels=3))
        self.assertFalse(result['valid'])
        self.assertEqual(result['used_channels'], [0, 2])
        self.assertTrue(self.bands(policy=SignalQualityOptions(min_channels=2))['valid'])
        self.invalid(lambda: self.bands(policy=SignalQualityOptions(min_channels=4)))

    def test_zero_power_is_invalid_even_if_flatline_rejection_is_disabled(self):
        result = self.bands(np.zeros_like(self.data), SignalQualityOptions(max_flatline_seconds=0))
        self.assertFalse(result['valid'])
        self.assertEqual(result['reason'], 'no_band_power')
        self.assertEqual(result['used_channels'], [0, 1, 2])
        self.assertIsNone(result['avg_band_powers'])

    def test_clipping_boundary_and_fraction_are_explicit(self):
        data = np.array([[-20., -10., 0., 10., 20.]])
        for allowed, flags in [(40, 0), (39.9, 2), (0, 2)]:
            result = DataFilter.get_signal_quality(data, [0], 128,
                SignalQualityOptions(clipping_bounds=(-20, 20), max_clipped_percentage=allowed))
            self.assertEqual(result['quality'][0]['clipped_percentage'], 40)
            self.assertEqual(result['quality'][0]['flags'], flags)
        disabled = DataFilter.get_signal_quality(data, [0], 128)
        self.assertIsNone(disabled['quality_policy']['clipping_bounds'])
        self.assertEqual(disabled['quality'][0]['flags'], 0)

    def test_flatline_measures_contiguous_time_not_just_fraction(self):
        data = np.array([[1., 1., 1., 2., 2., 2., 3., 3., 3., 4., 4.]])
        result = DataFilter.get_signal_quality(data, [0], 10,
            SignalQualityOptions(max_flatline_seconds=.3))
        q = result['quality'][0]
        self.assertEqual(q['flatline_percentage'], 70)
        self.assertEqual(q['longest_flatline_seconds'], .2)
        self.assertEqual(q['flags'], 0)
        result = DataFilter.get_signal_quality(data, [0], 10,
            SignalQualityOptions(max_flatline_seconds=.2))
        self.assertEqual(result['quality'][0]['flags'], int(SignalQualityFlags.FLATLINE))

    def test_nonfinite_breaks_flatline_runs_and_reasons_combine(self):
        data = np.array([[20., 20., 20., np.nan, 20., 20., 20.]])
        result = DataFilter.get_signal_quality(data, [0], 10,
            SignalQualityOptions(clipping_bounds=(-20, 20), max_flatline_seconds=.3))
        self.assertEqual(result['quality'][0]['flags'], 3)
        self.assertEqual(result['quality'][0]['longest_flatline_seconds'], .2)
        result = DataFilter.get_signal_quality(data, [0], 10,
            SignalQualityOptions(clipping_bounds=(-20, 20), max_flatline_seconds=.2))
        self.assertEqual(result['quality'][0]['reasons'], ['nonfinite', 'clipped', 'flatline'])

    def test_tolerance_and_flatline_disable(self):
        data = np.array([[0., .01, .02, .03, .04]])
        q = DataFilter.get_signal_quality(data, [0], 10,
            SignalQualityOptions(flatline_tolerance=.011, max_flatline_seconds=.3))['quality'][0]
        self.assertEqual(q['flags'], 4)
        q = DataFilter.get_signal_quality(data, [0], 10,
            SignalQualityOptions(flatline_tolerance=.011, max_flatline_seconds=0))['quality'][0]
        self.assertEqual(q['flags'], 0)
        self.assertEqual(q['flatline_percentage'], 100)

    def test_invalid_policies(self):
        for values in [dict(clipping_bounds=(1, -1)), dict(clipping_bounds=(0, np.inf)),
                       dict(clipping_bounds=(0,)), dict(max_clipped_percentage=-1),
                       dict(max_clipped_percentage=101), dict(max_clipped_percentage=np.nan),
                       dict(flatline_tolerance=-1), dict(flatline_tolerance=np.inf),
                       dict(max_flatline_seconds=-1), dict(max_flatline_seconds=np.nan),
                       dict(min_channels=0), dict(min_channels=True), dict(min_channels=1.5),
                       dict(min_channels=2**40)]:
            with self.subTest(values=values):
                self.invalid(lambda: self.bands(policy=SignalQualityOptions(**values)))

    def test_invalid_selection_and_shapes(self):
        for channels in ([], [0, 0], [-1], [3], [1.5], [True], [[0]]):
            with self.subTest(channels=channels):
                self.invalid(lambda: self.bands(channels=channels))
        for rate in (0, -1, True, 128.5, 2**32):
            self.invalid(lambda: DataFilter.get_signal_quality(self.data, [0], rate))
        self.invalid(lambda: DataFilter.get_signal_quality(np.zeros((1, 1)), [0], 128))

    def test_invalid_dsp_arguments_still_fail_when_all_channels_are_rejected(self):
        bad = np.zeros_like(self.data)
        for bands in ([], [(10, 5)], [(1, 100)], [(np.nan, 3)]):
            self.invalid(lambda: DataFilter.get_custom_band_powers_with_quality(bad, bands, [0, 1], 128))
        self.invalid(lambda: self.bands(np.zeros((3, 10))), BrainFlowExitCodes.INVALID_BUFFER_SIZE_ERROR)

    def test_custom_bands_and_unselected_bad_channels(self):
        self.data[1] = np.nan
        bands = [(5, 7), (9, 11)]
        result = DataFilter.get_custom_band_powers_with_quality(self.data, bands, [2, 0], 128, False)
        expected = DataFilter.get_custom_band_powers(self.data, bands, [2, 0], 128, False)
        np.testing.assert_array_equal(result['avg_band_powers'], expected[0])
        self.assertTrue(result['valid'])

    def test_native_abi_policy_size_version_and_null_checks(self):
        method = DataHandlerDLL.get_instance().get_signal_quality
        reports = (_NativeChannelQuality * 3)()
        p = self.policy._native()
        for field, value in [('struct_size', 0), ('version', 2), ('enable_clipping', 2)]:
            original = getattr(p, field)
            setattr(p, field, value)
            ec = method(self.data.ravel(), 3, self.data.shape[1], 128, ctypes.byref(p), reports)
            self.assertEqual(ec, BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
            setattr(p, field, original)
        for params, output in [(None, reports), (ctypes.byref(p), None)]:
            self.assertEqual(method(self.data.ravel(), 3, self.data.shape[1], 128, params, output),
                             BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)
        self.assertEqual(method(self.data.ravel(), 2147483647, 2, 128, ctypes.byref(p), reports),
                         BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)

    def test_independent_concurrent_calls_have_no_rejection_state(self):
        def calculate(i):
            data = self.data.copy()
            if i % 2:
                data[1] = 0
            return self.bands(data)['rejected_channels']
        with ThreadPoolExecutor(max_workers=4) as executor:
            result = list(executor.map(calculate, range(16)))
        self.assertEqual(result, [[1] if i % 2 else [] for i in range(16)])

    def test_native_invalid_result_clears_valid_and_powers(self):
        method = DataHandlerDLL.get_instance().get_custom_band_powers_with_quality
        data = np.zeros_like(self.data)
        starts = np.array([8.])
        stops = np.array([13.])
        means, variation = np.ones(1), np.ones(1)
        reports = (_NativeChannelQuality * 3)()
        params = self.policy._native()
        valid = ctypes.c_int(1)
        ec = method(data.ravel(), 3, data.shape[1], starts, stops, 1, 128, 0,
                    ctypes.byref(params), means, variation, reports, ctypes.byref(valid))
        self.assertEqual(ec, 0)
        self.assertEqual(valid.value, 0)
        self.assertTrue(np.isnan(means).all() and np.isnan(variation).all())
        self.assertTrue(all(q.flags == int(SignalQualityFlags.FLATLINE) for q in reports))

    def test_transition_logger_does_not_repeat_rejections_and_logs_recovery(self):
        logger = QualityTransitionLogger()
        clean = self.bands()
        self.data[1, 100] = 1000
        bad = self.bands()
        with patch.object(DataFilter, 'log_message') as log:
            logger.update(clean, 0)
            logger.update(bad, 100)
            logger.update(bad, 200)
            logger.update(clean, 300)
        self.assertEqual(log.call_count, 2)
        events = [json.loads(call.args[1]) for call in log.call_args_list]
        self.assertEqual([e['state'] for e in events], ['rejected', 'recovered'])
        self.assertEqual(events[0]['channel'], 1)
        self.assertEqual(events[0]['reasons'], ['clipped'])


if __name__ == '__main__':
    unittest.main()
