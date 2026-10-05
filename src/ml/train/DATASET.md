# Recording data and feature caches

The training directory contains `relaxed/` and `focused/` class directories. These
directory names are the label source: relaxed is `0`, focused is `1`. Recordings
are headerless, tab-separated numeric files with one **sample per line** and one
board channel per column, matching the files written by `DataFilter.write_file`.
The `.csv` extension does not imply comma separators. In memory, BrainFlow uses
the transpose: channels by samples.

Two layouts are supported:

```text
data/relaxed/recording.csv
data/focused/recording.csv

data/relaxed/7/recording.csv
data/focused/7/recording.csv
```

For the flat layout, supply a board ID explicitly or provide a manifest with board
metadata. For the legacy nested layout, the numeric parent directory supplies the
board ID when no explicit ID is given. The loader does not guess board identity
from the number of columns. A manifest entry takes precedence over the default
board ID.

## Manifest

Paths are relative to the training data directory, use `/` separators, and must
refer to existing CSV files. Entries can cover only the recordings that need
overrides. The remaining recordings use the explicit default board ID or legacy
directory metadata.

```json
{
  "recordings": [
    {
      "path": "relaxed/recording.csv",
      "board_id": 7,
      "subject_id": "participant-01",
      "session_id": "visit-01"
    },
    {
      "path": "focused/recording.csv",
      "board_id": 7,
      "subject_id": "participant-01",
      "session_id": "visit-01"
    }
  ]
}
```

`subject_id` keeps all recordings from that subject together, including different
sessions and labels. Without a subject ID, `session_id` is used. An explicit
`group_id` overrides both. Without any of these fields, the loader groups by the
UTC date of the first valid Unix timestamp. Use stable globally unique IDs; do not
reuse a session ID for unrelated sessions. Filename suffixes are not interpreted
as subject or session identifiers.

Optional `sampling_rate`, `eeg_channels`, and `timestamp_channel` fields override
the board descriptor. Channel indices are zero-based board columns. A manifest
without a board ID must provide all three fields. For example, an explicitly
described four-channel recording can use:

```json
{
  "recordings": [
    {
      "path": "focused/custom.csv",
      "sampling_rate": 250,
      "eeg_channels": [1, 2, 3, 4],
      "timestamp_channel": 10,
      "subject_id": "participant-02"
    }
  ]
}
```

## Feature extraction

`training_data.extract_dataset` returns a `FeatureDataset` containing `X` (five
float64 band powers), `y`, `groups`, `recording_ids`, `augmented`, and JSON-compatible
metadata. It calls the current `DataFilter.get_avg_band_powers` implementation,
with bands 2–4, 4–8, 8–13, 13–30, and 30–45 Hz.

Defaults skip the first 10 seconds, distribute nonoverlapping eight-second
canonical windows across each recording, and cap them at 128 per recording. Extra
five- and twelve-second windows are marked `augmented=True`. Their cap is applied
separately to each duration. Optional channel subsets also produce augmented
rows. All augmented rows must remain in training folds; validation and test
scores use canonical rows only. Every row retains its recording and group ID.

The loader queries the native filter guards before extracting features. A window
must include the required margins and retained FFT segment. At 250 Hz with the
current defaults, the minimum is 1,172 samples (4.688 seconds), including 330
discarded samples at each edge. A shorter augmentation duration is reported and
skipped without discarding valid windows from the recording. Nonfinite signal
windows, zero total power, parsing errors, and native errors are logged and
included in `metadata["issues"]`; missing files are never silently ignored.

Feature caches are compressed NPZ files containing numeric/string arrays and JSON
metadata. Loading never enables pickle. Cache fingerprints include relative
source paths, file sizes and modification times, extraction options, manifest
contents, resolved board/channel metadata, native guard settings, and SHA-256
hashes of the DSP binary, Python DSP wrapper, and extractor. A mismatch triggers
fresh extraction. If external tooling edits a raw file while preserving both its
size and nanosecond modification time, delete the cache to force regeneration.
Cache files and raw data are local artifacts and should not be committed.

## Current local collection

The inspected collection contains 50 recordings: 29 focused recordings (about
4.90 hours at nominal 250 Hz) and 21 relaxed recordings (about 10.49 hours). Their
12-column schema matches BrainBit board 7: EEG columns 1–4, counter column 0,
resistance columns 5–8, battery column 9, timestamp column 10, and marker column 11.
The user has confirmed that this collection uses BrainBit board 7. Supply that
board ID explicitly because the files do not themselves store it. Subject and
session identifiers have not been provided.

Most long-interval timestamp rates agree with nominal 250 Hz. Several older
focused recordings have substantial timestamp batching or clock drift; adjacent
timestamp spacing is not a reliable sampling-rate estimate. Thirty-six older
files end with a zero timestamp despite populated signal data. Group inference
ignores invalid timestamps, while feature extraction uses the configured nominal
rate and sample order.

The collection has 25 UTC recording-day groups. Focused recordings span six days
in 2020; relaxed recordings span 20 days, including 14 files from 2022. Only one
day contains both labels. Day, era, and recording setup can therefore confound
classification. Holding out complete days prevents overlap leakage but does not
demonstrate generalization to new subjects. Report grouped metrics and the
2020-only sensitivity analysis alongside aggregate scores. Acquire explicit
subject/session metadata and both labels across comparable recording conditions
for stronger validation.

The inspection found no identical complete recordings, identical complete EEG
arrays, or repeated aligned ten-second EEG blocks. A limited cross-file
correlation screen found strong shared 50 Hz contamination; correlation alone
does not establish duplicated recordings or common subjects. The detailed local
inventory is `build/reviews/training_data_inventory.json`.
