"""Python ML buffer ownership and input-layout regressions; no native model needed."""
import json
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'python_package'))
from brainflow.board_shim import BrainFlowError
from brainflow.ml_model import BrainFlowModelParams, MLModel, MLModuleDLL


class MLBindingRegression(unittest.TestCase):
    def test_output_capacity_matches_initial_serialized_settings(self):
        params = BrainFlowModelParams(0, 0)
        params.max_array_size = 4
        model = MLModel(params)
        params.max_array_size = 1

        def predict(data, size, output, output_len, serialized):
            self.assertEqual(json.loads(serialized)['max_array_size'], 4)
            self.assertEqual(len(output), 4)
            output[:] = [0.1, 0.2, 0.3, 0.4]
            output_len[0] = 4
            return 0

        with patch.object(MLModuleDLL, 'get_instance') as library:
            library.return_value.predict.side_effect = predict
            np.testing.assert_array_equal(model.predict([1, 2, 3, 4, 5]), [0.1, 0.2, 0.3, 0.4])

    def test_capacity_must_be_a_positive_native_integer(self):
        for capacity in (0, -1, 1.5, True, np.bool_(True), '4', 2 ** 31):
            params = BrainFlowModelParams(0, 0)
            params.max_array_size = capacity
            with self.subTest(capacity=capacity), self.assertRaises(BrainFlowError):
                MLModel(params)
        params = BrainFlowModelParams(0, 0)
        params.max_array_size = np.int64(4)
        self.assertEqual(json.loads(MLModel(params).serialized_params)['max_array_size'], 4)

    def test_strided_input_is_normalized_before_native_access(self):
        model = MLModel(BrainFlowModelParams(0, 0))
        values = np.arange(10, dtype=np.float32)[::-2]

        def predict(data, size, output, output_len, serialized):
            self.assertEqual(data.dtype, np.float64)
            self.assertTrue(data.flags.c_contiguous)
            np.testing.assert_array_equal(data, values)
            self.assertEqual(size, 5)
            output[0], output_len[0] = 0.5, 1
            return 0

        with patch.object(MLModuleDLL, 'get_instance') as library:
            library.return_value.predict.side_effect = predict
            np.testing.assert_array_equal(model.predict(values), [0.5])

    def test_invalid_inputs_do_not_reach_native_code(self):
        model = MLModel(BrainFlowModelParams(0, 0))
        with patch.object(MLModuleDLL, 'get_instance') as library:
            for values in ([], 1.0, [[1, 2]], [1, np.nan], [np.inf], [1 + 2j], ['not numeric']):
                with self.subTest(values=values), self.assertRaises(BrainFlowError):
                    model.predict(values)
            library.assert_not_called()

    def test_invalid_native_output_count_is_rejected(self):
        params = BrainFlowModelParams(0, 0)
        params.max_array_size = 2
        model = MLModel(params)
        for invalid_count in (-1, 3):
            def predict(data, size, output, output_len, serialized):
                output_len[0] = invalid_count
                return 0
            with patch.object(MLModuleDLL, 'get_instance') as library:
                library.return_value.predict.side_effect = predict
                with self.subTest(count=invalid_count), self.assertRaises(BrainFlowError):
                    model.predict([1, 2, 3, 4, 5])


if __name__ == '__main__':
    unittest.main()
