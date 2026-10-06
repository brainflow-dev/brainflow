"""Recording/group/cache regressions using small synthetic BrainBit-layout files."""
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / "python_package"))
from training_data import FeatureDataset, extract_dataset


class TrainingDataTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        for label in ("relaxed", "focused"):
            (self.root / label).mkdir()
            self.write_recording(label, seconds=20)

    def tearDown(self):
        self.temporary.cleanup()

    def write_recording(self, label, seconds, timestamp=1700000000):
        count = seconds * 250
        times = np.arange(count) / 250
        data = np.zeros((count, 12))
        data[:, 0] = np.arange(1, count + 1)
        for channel in range(1, 5):
            data[:, channel] = 1000 + np.sin(2 * np.pi * (channel + 7) * times)
        data[:, 9] = 80
        data[:, 10] = timestamp + times
        data[-1, 10] = 0  # Old recordings have an invalid final timestamp.
        np.savetxt(self.root / label / "record.csv", data, delimiter="\t")

    def extract(self, **kwargs):
        return extract_dataset(self.root, board_id=7, workers=1, **kwargs)

    def test_flat_files_require_explicit_board(self):
        with self.assertRaisesRegex(ValueError, "require.*board"):
            extract_dataset(self.root)

    def test_short_augmentation_does_not_discard_valid_recording(self):
        result = self.extract()
        self.assertEqual(result.X.shape, (6, 5))
        self.assertEqual(np.count_nonzero(~result.augmented), 2)
        self.assertEqual(len(set(result.groups)), 1)  # Both labels on one day stay together.
        self.assertEqual(set(result.y), {0, 1})
        for record in result.metadata["recordings"]:
            self.assertEqual(record["invalid_timestamp_count"], 1)
            self.assertEqual(record["canonical_windows"], 1)
            self.assertIn("recording_too_short_for_12s_after_initial_skip", record["skipped_windows"])

    def test_manifest_subjects_override_date_and_subsets_are_training_only(self):
        manifest = {"recordings": [
            {"path": f"{label}/record.csv", "subject_id": "person-a"} for label in ("relaxed", "focused")
        ]}
        result = self.extract(manifest=manifest, augmentation_windows=(), channel_subsets=True)
        self.assertEqual(set(result.groups), {"subject:person-a"})
        self.assertEqual(np.count_nonzero(~result.augmented), 2)
        self.assertEqual(np.count_nonzero(result.augmented), 8)

    def test_canonical_window_must_include_native_filter_guard(self):
        with self.assertRaisesRegex(ValueError, "including filter guards"):
            self.extract(window_seconds=0.5)

    def test_bad_recording_is_reported_and_other_recording_survives(self):
        (self.root / "relaxed" / "broken.csv").write_text("not numeric\n", encoding="utf-8")
        result = self.extract(augmentation_windows=())
        failures = [issue for issue in result.metadata["issues"] if issue["status"] == "failed"]
        self.assertEqual(len(failures), 1)
        self.assertEqual(failures[0]["recording_id"], "relaxed/broken.csv")
        self.assertEqual(result.X.shape, (2, 5))

    def test_cache_is_pickle_free_and_invalidates_on_source_changes(self):
        cache = self.root / "features.npz"
        result = self.extract(cache_path=cache, augmentation_windows=())
        loaded = FeatureDataset.load(cache)
        np.testing.assert_array_equal(result.X, loaded.X)
        with patch("training_data._extract_recording", side_effect=AssertionError("Cache was not reused")):
            reused = self.extract(cache_path=cache, augmentation_windows=())
            self.assertEqual(result.metadata["fingerprint"], reused.metadata["fingerprint"])
        source = self.root / "focused" / "record.csv"
        stat = source.stat()
        os.utime(source, ns=(stat.st_atime_ns, stat.st_mtime_ns + 1000000))
        refreshed = self.extract(cache_path=cache, augmentation_windows=())
        self.assertNotEqual(result.metadata["fingerprint"], refreshed.metadata["fingerprint"])
        with np.load(cache, allow_pickle=False) as contents:
            self.assertTrue(all(contents[key].dtype.kind != "O" for key in contents.files))


if __name__ == "__main__":
    unittest.main()
