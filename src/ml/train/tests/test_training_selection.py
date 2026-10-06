"""End-to-end checks that recording groups and holdout data remain isolated."""
from pathlib import Path
import sys
import unittest

import numpy as np

ROOT = Path(__file__).resolve().parents[4]
sys.path[:0] = [str(ROOT / "python_package"), str(ROOT / "src/ml/train")]
from training_data import FeatureDataset
from train_classifiers import evaluate_candidate, freeze_splits, score_predictions


def dataset_fixture():
    generator = np.random.default_rng(915)
    X, y, groups, recordings, augmented = [], [], [], [], []
    for day in range(12):
        for label in (0, 1):
            values = generator.dirichlet(np.ones(5), size=4)
            values[:, label] += 0.4
            values /= values.sum(axis=1, keepdims=True)
            # Augmentations intentionally duplicate canonical observations, so
            # accidentally scoring them would reproduce overlapping-window leakage.
            X.extend(np.concatenate((values, values[:3])))
            y.extend([label] * 7)
            groups.extend([f"day-{day:02}"] * 7)
            recordings.extend([f"day-{day:02}/class-{label}"] * 7)
            augmented.extend([False] * 4 + [True] * 3)
    return FeatureDataset(np.asarray(X), np.asarray(y), np.asarray(groups),
                          np.asarray(recordings), np.asarray(augmented), {}).validate()


def take_rows(data, rows):
    return FeatureDataset(data.X[rows], data.y[rows], data.groups[rows],
                          data.recording_ids[rows], data.augmented[rows], {}).validate()


class TrainingSelectionTests(unittest.TestCase):
    def test_group_holdout_and_cv_keep_all_augmentations_on_training_side(self):
        data = dataset_fixture()
        development, holdout, folds, _ = freeze_splits(data, cv_folds=3, holdout_folds=4)
        holdout_groups = set(data.groups[holdout])
        self.assertFalse(holdout_groups.intersection(data.groups[development]))
        self.assertFalse(data.augmented[holdout].any())
        validation_rows = []
        for train, validation in folds:
            self.assertFalse(set(data.groups[train]).intersection(data.groups[validation]))
            self.assertFalse(holdout_groups.intersection(data.groups[train]))
            self.assertFalse(holdout_groups.intersection(data.groups[validation]))
            self.assertTrue(data.augmented[train].any())
            self.assertFalse(data.augmented[validation].any())
            self.assertEqual(set(data.y[train]), {0, 1})
            self.assertEqual(set(data.y[validation]), {0, 1})
            validation_rows.extend(validation)
        expected = development[~data.augmented[development]]
        np.testing.assert_array_equal(np.sort(validation_rows), np.sort(expected))
        self.assertEqual(len(validation_rows), len(set(validation_rows)))

    def test_split_assignment_is_invariant_to_recording_duration(self):
        data = dataset_fixture()
        description = freeze_splits(data, cv_folds=3, holdout_folds=4)[3]
        selected = data.recording_ids == data.recording_ids[0]
        # One much longer recording must not alter representative-level splits.
        rows = np.concatenate((np.arange(len(data.X)), np.tile(np.flatnonzero(selected), 40)))
        longer = take_rows(data, rows)
        modified = freeze_splits(longer, cv_folds=3, holdout_folds=4)[3]
        for field in ("development_groups", "holdout_groups", "development_recordings", "holdout_recordings"):
            self.assertEqual(description[field], modified[field])
        for original, changed in zip(description["folds"], modified["folds"]):
            for field in ("train_groups", "validation_groups", "train_recordings", "validation_recordings"):
                self.assertEqual(original[field], changed[field])

    def test_candidate_scores_do_not_read_heldout_features(self):
        data = dataset_fixture()
        _, holdout, folds, _ = freeze_splits(data, cv_folds=3, holdout_folds=4)
        spec = dict(name="isolation-check", family="logreg", params={"C": 0.7}, augmentation=True)
        before = evaluate_candidate(spec, data.X, data.y, data.groups, data.recording_ids,
                                    data.augmented, folds, seed=42)
        self.assertEqual(before["status"], "ok")
        changed = data.X.copy()
        # Include held-out augmented rows as well as the canonical scoring rows.
        hidden = np.isin(data.groups, data.groups[holdout])
        changed[hidden] = np.nan
        after = evaluate_candidate(spec, changed, data.y, data.groups, data.recording_ids,
                                   data.augmented, folds, seed=42)
        self.assertEqual(after["status"], "ok")
        self.assertEqual(before["mean_macro_f1"], after["mean_macro_f1"])
        self.assertEqual(before["pooled"], after["pooled"])
        self.assertEqual(before["folds"], after["folds"])

    def test_recording_weighted_metrics_are_invariant_to_duplicate_windows(self):
        y = np.array([0, 0, 0, 0, 1, 1, 1, 1])
        probability = np.array([0.1, 0.4, 0.6, 0.2, 0.4, 0.8, 0.6, 0.9])
        recordings = np.repeat(["a", "b", "c", "d"], 2)
        original = score_predictions(y, probability, recordings)
        rows = np.concatenate((np.arange(len(y)), np.tile(np.flatnonzero(recordings == "a"), 30)))
        duplicated = score_predictions(y[rows], probability[rows], recordings[rows])
        for level in ("window", "recording"):
            for name, value in original[level].items():
                np.testing.assert_allclose(value, duplicated[level][name], rtol=0, atol=1e-13)


if __name__ == "__main__":
    unittest.main()
