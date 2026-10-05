"""Small group-safety and deployment tests, independent of the EEG recordings."""
import json
from pathlib import Path
import sys
import unittest

import numpy as np
from onnx import checker
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType
from threadpoolctl import threadpool_limits

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from training_models import (build_model, build_stacking, candidate_specs,
                             fit_model, grouped_splits, _recording_weights, _uniform_recording_indices)


class TrainingModelsTests(unittest.TestCase):
    def setUp(self):
        self.recordings = np.repeat(np.arange(12), 12)
        self.y = np.repeat(np.arange(12) % 2, 12)
        self.groups = self.recordings.copy()
        self.X = np.random.default_rng(5).dirichlet(np.ones(5), size=len(self.y)).astype(np.float32)
        self.X[:, 0] += 0.15 * self.y
        self.X /= self.X.sum(axis=1, keepdims=True)

    def test_specs_are_json_and_deterministic(self):
        full = candidate_specs()
        self.assertEqual(json.loads(json.dumps(full)), full)
        self.assertEqual(full, candidate_specs())
        self.assertEqual(len({spec['name'] for spec in full}), len(full))
        self.assertLess(len(candidate_specs(quick=True)), len(full))

    def test_group_folds_cover_rows_once_without_leakage(self):
        folds = grouped_splits(self.y, self.groups)
        all_validation = []
        for training, validation in folds:
            self.assertFalse(set(self.groups[training]) & set(self.groups[validation]))
            self.assertEqual(set(self.y[training]), {0, 1})
            self.assertEqual(set(self.y[validation]), {0, 1})
            all_validation.extend(validation)
        np.testing.assert_array_equal(np.sort(all_validation), np.arange(len(self.y)))

    def test_group_folds_fallback_without_sample_level_cv(self):
        groups = np.repeat(np.arange(4), 5)
        labels = np.repeat([0, 0, 1, 1], 5)
        self.assertEqual(len(grouped_splits(labels, groups, n_splits=4)), 2)
        with self.assertRaises(ValueError):
            grouped_splits(np.repeat([0, 1], 5), np.repeat([0, 1], 5))

    def test_recording_class_weights_do_not_follow_duration(self):
        recording_ids = np.repeat(np.arange(5), [3, 10, 30, 7, 20])
        labels = np.repeat([0, 0, 1, 1, 1], [3, 10, 30, 7, 20])
        weights = _recording_weights(labels, recording_ids)
        self.assertAlmostEqual(weights[labels == 0].sum(), weights[labels == 1].sum())
        self.assertAlmostEqual(weights[recording_ids == 0].sum(), weights[recording_ids == 1].sum())
        self.assertAlmostEqual(weights[recording_ids == 2].sum(), weights[recording_ids == 3].sum())

    def assertExport(self, model):
        predictions = model.predict_proba(self.X)
        self.assertEqual(predictions.shape, (len(self.y), 2))
        self.assertTrue(np.isfinite(predictions).all())
        np.testing.assert_allclose(predictions.sum(axis=1), 1., atol=1e-6)
        final = model.steps[-1][1] if hasattr(model, 'steps') else model
        graph = convert_sklearn(model, initial_types=[('mindfulness_input', FloatTensorType([1, 5]))],
                                target_opset=11, options={id(final): {'zipmap': False}})
        checker.check_model(graph)
        self.assertEqual(graph.graph.input[0].type.tensor_type.shape.dim[1].dim_value, 5)
        self.assertIn('probabilities', [output.name for output in graph.graph.output])

    def test_short_recording_does_not_shrink_all_training_data(self):
        records = np.repeat(np.arange(4), [13, 128, 128, 128])
        labels = np.repeat([0, 0, 1, 1], [13, 128, 128, 128])
        selected = _uniform_recording_indices(labels, records)
        self.assertEqual(len(selected), 4 * 128)
        self.assertEqual(len(np.unique(selected[records[selected] == 0])), 13)
        for recording in range(1, 4):
            self.assertEqual(len(np.unique(selected[records[selected] == recording])), 128)

    def test_every_family_fits_and_exports(self):
        selected = {}
        for spec in candidate_specs(quick=True):
            selected.setdefault(spec['family'], spec)
        with threadpool_limits(limits=1):
            for family, spec in selected.items():
                with self.subTest(family=family):
                    model = fit_model(build_model(spec, self.X, self.y, self.groups),
                                      self.X, self.y, self.recordings)
                    self.assertExport(model)

    def test_stacking_fits_and_exports_with_group_cv(self):
        specs = candidate_specs(quick=True)
        chosen = [next(spec for spec in specs if spec['family'] == family)
                  for family in ('logreg', 'random_forest', 'knn', 'mlp')]
        with threadpool_limits(limits=1):
            model = fit_model(build_stacking(chosen, self.X, self.y, self.groups),
                              self.X, self.y, self.recordings)
            self.assertExport(model)


if __name__ == '__main__':
    unittest.main()
