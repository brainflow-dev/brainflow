"""Unit tests for DataFilter.rereference_channels."""

import unittest

import numpy as np

from brainflow.data_filter import DataFilter
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes


class RereferenceChannelsTest(unittest.TestCase):
    def setUp(self):
        self.data = np.array([
            [1.0, 2.0, 3.0],
            [4.0, 5.0, 6.0],
            [7.0, 8.0, 9.0],
            [10.0, 11.0, 12.0],
        ], dtype=np.float64)

    def assert_invalid(self, data, channels, reference_channels):
        with self.assertRaises(BrainFlowError) as error:
            DataFilter.rereference_channels(data, channels, reference_channels)
        self.assertEqual(error.exception.exit_code, BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value)

    def test_average_reference_across_two_channels(self):
        data = self.data.copy()
        DataFilter.rereference_channels(data, [0, 1, 2, 3], [0, 1])
        reference = np.array([2.5, 3.5, 4.5])
        np.testing.assert_allclose(data, self.data - reference)

    def test_average_reference_across_three_channels(self):
        data = self.data.copy()
        DataFilter.rereference_channels(data, [0, 1, 2, 3], [0, 1, 2])
        reference = np.array([4.0, 5.0, 6.0])
        np.testing.assert_allclose(data, self.data - reference, atol=1e-12)

    def test_reference_to_single_channel(self):
        data = self.data.copy()
        DataFilter.rereference_channels(data, [0, 1, 2, 3], [1])
        np.testing.assert_allclose(data, self.data - self.data[1])
        np.testing.assert_array_equal(data[1], np.zeros(3))

    def test_selected_channels_only(self):
        data = self.data.copy()
        DataFilter.rereference_channels(data, [0, 1], [0, 1])
        reference = np.array([2.5, 3.5, 4.5])
        expected = self.data.copy()
        expected[0] -= reference
        expected[1] -= reference
        np.testing.assert_allclose(data, expected)
        np.testing.assert_array_equal(data[2], self.data[2])
        np.testing.assert_array_equal(data[3], self.data[3])

    def test_common_average_reference_allows_overlap(self):
        data = self.data.copy()
        DataFilter.rereference_channels(data, [0, 1, 2, 3], [0, 1, 2, 3])
        reference = np.mean(self.data, axis=0)
        np.testing.assert_allclose(data, self.data - reference)

    def test_output_shape_equals_input_shape(self):
        data = self.data.copy()
        DataFilter.rereference_channels(data, [0, 1], [0, 1])
        self.assertEqual(data.shape, self.data.shape)

    def test_invalid_channel_index_raises(self):
        self.assert_invalid(self.data.copy(), [0, 4], [0])
        self.assert_invalid(self.data.copy(), [0], [4])
        self.assert_invalid(self.data.copy(), [-1], [0])

    def test_empty_channel_lists_raise(self):
        self.assert_invalid(self.data.copy(), [], [0])
        self.assert_invalid(self.data.copy(), [0], [])

    def test_duplicate_channel_indices_raise(self):
        self.assert_invalid(self.data.copy(), [0, 1, 0], [1])
        self.assert_invalid(self.data.copy(), [0], [1, 1])

    def test_non_2d_input_raises(self):
        self.assert_invalid(np.array([1.0, 2.0, 3.0], dtype=np.float64), [0], [0])

    def test_sliced_channel_index_arrays(self):
        data = self.data.copy()
        channels = np.arange(4, dtype=np.int32)[::2]
        DataFilter.rereference_channels(data, channels, [3])
        expected = self.data.copy()
        expected[[0, 2]] -= self.data[3]
        np.testing.assert_allclose(data, expected)

        data = self.data.copy()
        reference = np.arange(4, dtype=np.int32)[::2]
        DataFilter.rereference_channels(data, [0, 1, 2, 3], reference)
        expected = self.data - np.mean(self.data[[0, 2]], axis=0)
        np.testing.assert_allclose(data, expected)

    def test_non_integer_and_overflowing_indices_raise(self):
        self.assert_invalid(self.data.copy(), [0.9], [0])
        self.assert_invalid(self.data.copy(), [-0.5], [0])
        self.assert_invalid(self.data.copy(), np.array([4294967296], dtype=np.int64), [0])
        self.assert_invalid(self.data.copy(), [0], [0.9])
        self.assert_invalid(self.data.copy(), [0], [-0.5])
        self.assert_invalid(self.data.copy(), [0], np.array([4294967296], dtype=np.int64))


if __name__ == '__main__':
    unittest.main()
