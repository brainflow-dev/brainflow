"""Deterministic, recording-aware extraction of the five runtime band-power features.

Flat directories require an explicit board ID. A manifest can supply per-recording
board/subject/session metadata; filenames and signal similarity never imply subjects.
Cache files contain ordinary NumPy arrays and JSON metadata, never pickle objects.
"""
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass
from datetime import datetime, timezone
import hashlib
import json
import logging
import os
from pathlib import Path
import tempfile

import numpy as np

from brainflow.board_shim import BoardShim
from brainflow.data_filter import DataFilter, DataHandlerDLL


FEATURE_SCHEMA = "avg-band-powers-2-4_4-8_8-13_13-30_30-45-v1"
LABELS = {"relaxed": 0, "focused": 1}


def _json(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False)


def _sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


@dataclass
class FeatureDataset:
    X: np.ndarray
    y: np.ndarray
    groups: np.ndarray
    recording_ids: np.ndarray
    augmented: np.ndarray
    metadata: dict

    def validate(self):
        self.X = np.asarray(self.X, dtype=np.float64)
        self.y = np.asarray(self.y, dtype=np.int64)
        self.groups = np.asarray(self.groups, dtype=str)
        self.recording_ids = np.asarray(self.recording_ids, dtype=str)
        self.augmented = np.asarray(self.augmented, dtype=bool)
        count = len(self.X)
        if self.X.ndim != 2 or self.X.shape[1] != 5 or not np.isfinite(self.X).all():
            raise ValueError("Features must be a finite N x 5 float64 array")
        for values in (self.y, self.groups, self.recording_ids, self.augmented):
            if values.shape != (count,):
                raise ValueError("Dataset arrays must have the same number of rows")
        if not np.isin(self.y, [0, 1]).all() or np.any(self.groups == "") or np.any(self.recording_ids == ""):
            raise ValueError("Invalid labels or missing recording/group identifiers")
        return self

    def save(self, path):
        self.validate()
        target = Path(path)
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=target.parent, suffix=".npz", delete=False) as stream:
                temporary = Path(stream.name)
                np.savez_compressed(stream, X=self.X, y=self.y, groups=self.groups,
                                    recording_ids=self.recording_ids, augmented=self.augmented,
                                    metadata=np.asarray(_json(self.metadata)))
            os.replace(temporary, target)
        finally:
            if temporary is not None and temporary.exists():
                temporary.unlink()

    @classmethod
    def load(cls, path):
        with np.load(path, allow_pickle=False) as cache:
            dataset = cls(X=cache["X"], y=cache["y"], groups=cache["groups"],
                          recording_ids=cache["recording_ids"], augmented=cache["augmented"],
                          metadata=json.loads(str(cache["metadata"].item())))
        return dataset.validate()


def save_dataset(dataset, path):
    dataset.save(path)


def load_dataset(path):
    return FeatureDataset.load(path)


def _read_manifest(manifest):
    if manifest is None:
        return {}, {}
    document = json.loads(Path(manifest).read_text(encoding="utf-8")) if isinstance(manifest, (str, Path)) else manifest
    entries = document.get("recordings", document) if isinstance(document, dict) else document
    if isinstance(entries, list):
        indexed = {}
        for entry in entries:
            key = entry.get("path", entry.get("recording_id"))
            if not key or str(key).replace("\\", "/") in indexed:
                raise ValueError("Every manifest recording needs a unique path")
            indexed[str(key).replace("\\", "/")] = dict(entry)
    elif isinstance(entries, dict):
        indexed = {str(key).replace("\\", "/"): dict(value) for key, value in entries.items()}
    else:
        raise ValueError("Manifest must contain a recording list or a path-keyed object")
    return indexed, document


def _descriptor(path, root, board_id, manifest):
    relative = path.relative_to(root).as_posix()
    parts = Path(relative).parts
    if len(parts) < 2 or parts[0] not in LABELS:
        raise ValueError(f"Unknown class directory for {relative}; expected relaxed/ or focused/")
    entry = manifest.get(relative, {})
    selected_board = entry.get("board_id", board_id)
    if selected_board is None and len(parts) >= 3:
        try:
            selected_board = int(parts[-2])
        except ValueError:
            pass
    if selected_board is None and not all(key in entry for key in ("sampling_rate", "eeg_channels", "timestamp_channel")):
        raise ValueError(f"{relative}: flat recordings require --board-id or manifest board metadata")
    if selected_board is not None:
        selected_board = int(selected_board)
        description = BoardShim.get_board_descr(selected_board)
    else:
        description = {}
    rate = int(entry.get("sampling_rate", description.get("sampling_rate", 0)))
    channels = entry.get("eeg_channels", description.get("eeg_channels", []))
    if rate <= 0 or not isinstance(channels, (list, tuple)) or not channels:
        raise ValueError(f"{relative}: positive sampling_rate and EEG channels are required")
    if any(not isinstance(channel, (int, np.integer)) or channel < 0 for channel in channels) or len(set(channels)) != len(channels):
        raise ValueError(f"{relative}: EEG channels must be distinct nonnegative integer indices")
    timestamp_channel = entry.get("timestamp_channel", description.get("timestamp_channel"))
    explicit_group = None
    for key, prefix in (("group_id", "manifest"), ("subject_id", "subject"), ("session_id", "session")):
        if entry.get(key) is not None:
            explicit_group = f"{prefix}:{entry[key]}"
            break
    if timestamp_channel is None and explicit_group is None:
        raise ValueError(f"{relative}: grouping requires a timestamp channel or manifest group/subject/session")
    return dict(recording_id=relative, label=parts[0], y=LABELS[parts[0]], board_id=selected_board,
                sampling_rate=rate, eeg_channels=list(channels), timestamp_channel=timestamp_channel,
                expected_columns=description.get("num_rows"), explicit_group=explicit_group,
                manifest=entry)


def _starts(samples, skip, length, cap):
    count = min(cap, max(0, (samples - skip) // length))
    if count == 0:
        return np.empty(0, dtype=np.int64)
    last = samples - length
    if count == 1:
        return np.asarray([(skip + last) // 2], dtype=np.int64)
    # Since count <= available/length, even the uncapped windows do not overlap.
    return np.linspace(skip, last, count, dtype=np.int64)


def _extract_recording(task):
    path, descriptor, config, native_info = task
    record = dict(descriptor)
    record.pop("manifest", None)
    record["status"] = "failed"
    record["skipped_windows"] = {}
    features, augmentation = [], []

    def skip(reason, amount=1):
        record["skipped_windows"][reason] = record["skipped_windows"].get(reason, 0) + amount

    try:
        # BrainFlow's TSV files store samples as rows. np.loadtxt parses much faster
        # than the generic native CSV reader for this well-defined training format.
        disk_data = np.loadtxt(path, delimiter="\t", ndmin=2)
        rows, columns = disk_data.shape
        record.update(samples=rows, columns=columns)
        expected = descriptor["expected_columns"]
        if expected is not None and columns != expected:
            raise ValueError(f"Expected {expected} columns for board {descriptor['board_id']}, found {columns}")
        channels = descriptor["eeg_channels"]
        if max(channels) >= columns:
            raise ValueError("EEG channel index exceeds the file column count")
        rate = descriptor["sampling_rate"]
        timestamp_channel = descriptor["timestamp_channel"]
        valid_ts = np.empty(0)
        timestamp_indices = np.empty(0, dtype=np.int64)
        if timestamp_channel is not None:
            if not isinstance(timestamp_channel, int) or timestamp_channel < 0 or timestamp_channel >= columns:
                raise ValueError("Invalid timestamp channel")
            timestamps = disk_data[:, timestamp_channel]
            valid = np.isfinite(timestamps) & (timestamps > 946684800) & (timestamps < 4102444800)
            timestamp_indices = np.flatnonzero(valid)
            valid_ts = timestamps[valid]
            record["invalid_timestamp_count"] = int((~valid).sum())
        if valid_ts.size:
            day = datetime.fromtimestamp(float(valid_ts[0]), timezone.utc).date().isoformat()
            record.update(recording_day_utc=day, first_timestamp=float(valid_ts[0]), last_timestamp=float(valid_ts[-1]))
            if valid_ts.size > 1 and valid_ts[-1] > valid_ts[0]:
                record["timestamp_rate_hz"] = float((timestamp_indices[-1] - timestamp_indices[0]) / (valid_ts[-1] - valid_ts[0]))
                record["timestamp_nonpositive_steps"] = int(np.count_nonzero(np.diff(valid_ts) <= 0))
        elif descriptor["explicit_group"] is None:
            raise ValueError("No valid Unix timestamp for grouping; supply a manifest group")
        group = descriptor["explicit_group"] or f"utc-day:{record['recording_day_utc']}"
        record["group"] = group
        record["duration_seconds"] = rows / rate
        signal = np.ascontiguousarray(disk_data[:, channels].T)
        del disk_data
        record["nonfinite_eeg_values"] = int(np.count_nonzero(~np.isfinite(signal)))
        skip_samples = int(round(config["skip_seconds"] * rate))
        selected = list(range(len(channels)))
        subsets = []
        if config["channel_subsets"] is True:
            subsets = [[index for index in selected if index != omitted] for omitted in selected] if len(selected) > 1 else []
        elif config["channel_subsets"]:
            for subset in config["channel_subsets"]:
                if not subset or any(channel not in channels for channel in subset) or len(set(subset)) != len(subset):
                    raise ValueError("Augmentation subsets must contain distinct EEG channel indices from this board")
                if list(subset) != channels:
                    subsets.append([channels.index(channel) for channel in subset])
        windows = [(config["window_seconds"], False)]
        windows += [(seconds, True) for seconds in config["augmentation_windows"] if seconds != config["window_seconds"]]
        for seconds, is_augmented in windows:
            length = int(round(seconds * rate))
            if length < native_info["minimum_samples"]:
                skip(f"window_{seconds:g}s_below_native_minimum_{native_info['minimum_samples']}_samples")
                continue
            starts = _starts(rows, skip_samples, length, config["max_windows_per_recording"])
            if starts.size == 0:
                skip(f"recording_too_short_for_{seconds:g}s_after_initial_skip")
                continue
            for start in starts:
                window = np.ascontiguousarray(signal[:, start:start + length])
                # Channel subsets apply to canonical durations only: avoid a large
                # Cartesian expansion of durations and channel combinations.
                selections = [(selected, is_augmented)]
                if not is_augmented:
                    selections += [(subset, True) for subset in subsets]
                for selected_channels, augmented in selections:
                    try:
                        if not np.isfinite(window[selected_channels]).all():
                            skip("window_contains_nonfinite_eeg")
                            continue
                        powers, _ = DataFilter.get_avg_band_powers(window, selected_channels, rate, config["apply_filter"])
                        powers = np.asarray(powers, dtype=np.float64)
                        if powers.shape != (5,) or not np.isfinite(powers).all() or np.any(powers < 0):
                            skip("invalid_band_power_output")
                            continue
                        if powers.sum() <= 0:
                            skip("window_has_zero_total_band_power")
                            continue
                        features.append(powers)
                        augmentation.append(augmented)
                    except Exception as error:
                        skip(f"band_power_error:{type(error).__name__}:{error}")
        record["canonical_windows"] = sum(not flag for flag in augmentation)
        record["augmented_windows"] = sum(augmentation)
        record["status"] = "ok" if features else "skipped"
        if not features:
            record["reason"] = "No usable feature windows; see skipped_windows"
        return features, augmentation, record
    except Exception as error:
        record["reason"] = f"{type(error).__name__}: {error}"
        return [], [], record


def extract_dataset(data_root, board_id=None, manifest=None, window_seconds=8.0,
                    augmentation_windows=(5.0, 12.0), max_windows_per_recording=128,
                    seed=42, cache_path=None, workers=4, skip_seconds=10.0,
                    channel_subsets=None, apply_filter=True):
    """Extract canonical and training-only augmented feature rows.

    ``groups`` defaults to UTC recording day across both classes. Manifest fields
    ``group_id``, ``subject_id``, or ``session_id`` override it in that order.
    Manifest paths are relative to ``data_root``. ``eeg_channels`` and optional
    augmentation subsets use board row indices, not positions in the EEG list.
    Canonical windows are uniformly spread, nonoverlapping, and capped per file;
    all extra durations/subsets have augmented=True and must stay out of scoring.
    File/window failures are retained in metadata["issues"] and logged.
    """
    root = Path(data_root).resolve()
    if not root.is_dir():
        raise ValueError(f"Training data directory does not exist: {root}")
    if not isinstance(max_windows_per_recording, int) or max_windows_per_recording < 1 or workers < 1:
        raise ValueError("max_windows_per_recording and workers must be positive integers")
    if not np.isfinite(window_seconds) or window_seconds <= 0 or not np.isfinite(skip_seconds) or skip_seconds < 0:
        raise ValueError("Window must be positive and initial skip must be nonnegative")
    augmentation_windows = sorted(set(float(value) for value in augmentation_windows))
    if any(not np.isfinite(value) or value <= 0 for value in augmentation_windows):
        raise ValueError("Augmentation windows must be finite and positive")
    manifest_entries, manifest_document = _read_manifest(manifest)
    paths = sorted(root.rglob("*.csv"))
    if not paths:
        raise ValueError(f"No CSV recordings found in {root}")
    descriptors = [_descriptor(path, root, board_id, manifest_entries) for path in paths]
    unknown_manifest_paths = set(manifest_entries) - {descriptor["recording_id"] for descriptor in descriptors}
    if unknown_manifest_paths:
        raise ValueError(f"Manifest paths were not found under data_root: {sorted(unknown_manifest_paths)}")
    native_info = {rate: DataFilter.get_band_power_info(rate, bool(apply_filter))
                   for rate in sorted({descriptor["sampling_rate"] for descriptor in descriptors})}
    for rate, info in native_info.items():
        if round(window_seconds * rate) < info["minimum_samples"]:
            raise ValueError(f"Canonical window {window_seconds}s at {rate}Hz needs at least {info['minimum_samples']} samples including filter guards")
    config = dict(feature_schema=FEATURE_SCHEMA, board_id=board_id, window_seconds=float(window_seconds),
                  augmentation_windows=augmentation_windows, max_windows_per_recording=max_windows_per_recording,
                  seed=int(seed), skip_seconds=float(skip_seconds), channel_subsets=channel_subsets,
                  apply_filter=bool(apply_filter), labels=LABELS)
    source_files = []
    for path, descriptor in zip(paths, descriptors):
        stat = path.stat()
        source_files.append(dict(path=descriptor["recording_id"], size=stat.st_size, mtime_ns=stat.st_mtime_ns))
    dll_path = Path(DataHandlerDLL.get_instance().lib._name).resolve()
    backend_path = Path(__import__(DataFilter.__module__, fromlist=["__file__"]).__file__).resolve()
    identity = dict(config=config, manifest=manifest_document, source_files=source_files,
                    resolved_recordings=descriptors, native_band_power_info=native_info,
                    source_fingerprint_mode="relative_path_size_mtime_ns",
                    native_dsp_sha256=_sha256(dll_path), python_dsp_sha256=_sha256(backend_path),
                    extractor_sha256=_sha256(__file__))
    fingerprint = hashlib.sha256(_json(identity).encode("utf-8")).hexdigest()
    if cache_path is not None and Path(cache_path).is_file():
        try:
            cached = FeatureDataset.load(cache_path)
            if cached.metadata.get("fingerprint") == fingerprint:
                logging.info("Reusing feature cache %s", cache_path)
                return cached
            logging.info("Feature cache fingerprint changed; extracting current data")
        except (ValueError, KeyError, OSError) as error:
            logging.warning("Ignoring unreadable feature cache %s: %s", cache_path, error)
    # Initialize all native singletons before launching threads. Each job owns its
    # NumPy data and filter state; output order stays tied to sorted file paths.
    tasks = [(path, descriptor, config, native_info[descriptor["sampling_rate"]])
             for path, descriptor in zip(paths, descriptors)]
    X, y, groups, recording_ids, augmented, recordings = [], [], [], [], [], []
    with ThreadPoolExecutor(max_workers=int(workers)) as pool:
        for values, flags, record in pool.map(_extract_recording, tasks):
            recordings.append(record)
            X.extend(values)
            y.extend([record["y"]] * len(values))
            groups.extend([record.get("group", "")] * len(values))
            recording_ids.extend([record["recording_id"]] * len(values))
            augmented.extend(flags)
            logging.info("%s: %s, %d canonical + %d augmented windows", record["recording_id"], record["status"],
                         record.get("canonical_windows", 0), record.get("augmented_windows", 0))
            if record["status"] != "ok" or record["skipped_windows"]:
                logging.warning("Recording issues: %s", _json(record))
    issues = [dict(recording_id=record["recording_id"], status=record["status"],
                   reason=record.get("reason"), skipped_windows=record["skipped_windows"])
              for record in recordings if record["status"] != "ok" or record["skipped_windows"]]
    if not X:
        raise ValueError("No usable training features: " + _json(issues))
    metadata = dict(fingerprint=fingerprint, config=config, identity=identity, recordings=recordings,
                    issues=issues, native_band_power_info={str(rate): info for rate, info in native_info.items()},
                    created_utc=datetime.now(timezone.utc).isoformat(), workers=int(workers),
                    grouping_caveat="UTC recording day is a conservative proxy, not evidence of independent subjects.")
    dataset = FeatureDataset(np.asarray(X, dtype=np.float64), np.asarray(y), np.asarray(groups),
                             np.asarray(recording_ids), np.asarray(augmented), metadata).validate()
    if cache_path is not None:
        dataset.save(cache_path)
    return dataset
