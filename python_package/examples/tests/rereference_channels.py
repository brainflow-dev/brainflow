"""Unit tests for DataFilter.rereference_channels."""

import unittest

import numpy as np

from brainflow.data_filter import DataFilter


class RereferenceChannelsTest(unittest.TestCase):
    def setUp(self):
        self.data = np.array([
            [1.0, 2.0, 3.0],
            [4.0, 5.0, 6.0],
            [7.0, 8.0, 9.0],
            [10.0, 11.0, 12.0],
        ], dtype=np.float64)

    def test_average_reference_across_two_channels(self):
        result = DataFilter.rereference_channels(self.data, [0, 1])
        reference = np.array([2.5, 3.5, 4.5])
        expected = self.data - reference
        np.testing.assert_allclose(result, expected)

    def test_average_reference_across_three_channels(self):
        result = DataFilter.rereference_channels(self.data, [0, 1, 2])
        reference = np.array([4.0, 5.0, 6.0])
        expected = self.data - reference
        np.testing.assert_allclose(result, expected)

    def test_reference_to_single_channel(self):
        result = DataFilter.rereference_channels(self.data, [1])
        expected = self.data - self.data[1]
        np.testing.assert_allclose(result, expected)
        np.testing.assert_array_equal(result[1], np.zeros(3))

    def test_multiple_reference_channels(self):
        result = DataFilter.rereference_channels(self.data, [0, 2, 3])
        reference = np.mean(self.data[[0, 2, 3]], axis=0)
        expected = self.data - reference
        np.testing.assert_allclose(result, expected)

    def test_excluded_channels_remain_unchanged(self):
        result = DataFilter.rereference_channels(self.data, [0, 1], excluded_channels=[2, 3])
        reference = np.array([2.5, 3.5, 4.5])
        expected = self.data.copy()
        expected[0] -= reference
        expected[1] -= reference
        np.testing.assert_allclose(result, expected)
        np.testing.assert_array_equal(result[2], self.data[2])
        np.testing.assert_array_equal(result[3], self.data[3])

    def test_input_array_is_not_mutated(self):
        original = self.data.copy()
        DataFilter.rereference_channels(self.data, [0, 1], excluded_channels=[3])
        np.testing.assert_array_equal(self.data, original)

    def test_output_shape_equals_input_shape(self):
        result = DataFilter.rereference_channels(self.data, [0, 1])
        self.assertEqual(result.shape, self.data.shape)

    def test_invalid_channel_index_raises(self):
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(self.data, [0, 4])
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(self.data, [0], excluded_channels=[4])
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(self.data, [-1])

    def test_empty_reference_channels_raises(self):
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(self.data, [])

    def test_duplicate_reference_channels_raise(self):
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(self.data, [0, 1, 0])

    def test_channel_cannot_be_both_reference_and_excluded(self):
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(self.data, [0, 1], excluded_channels=[1])

    def test_non_2d_input_raises(self):
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(np.array([1.0, 2.0, 3.0]), [0])
        with self.assertRaises(ValueError):
            DataFilter.rereference_channels(np.zeros((2, 2, 2)), [0])


if __name__ == '__main__':
    unittest.main()
