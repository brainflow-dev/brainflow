# Classifier training

Run training from the repository root with Python 3.11 and the current checkout's
Python binding and native libraries. The classifier inputs are the five normalized
band powers produced by the current `DataFilter.get_avg_band_powers`: 2–4, 4–8,
8–13, 13–30, and 30–45 Hz. Relaxed is class 0; focused/mindfulness is class 1.

## Setup

Use a dedicated Python 3.11 environment and the pinned training/export packages:

```sh
python -B -m pip install -r src/ml/train/requirements.txt
python -B -m pip install --no-deps -e python_package
python tools/build.py --onnx
```

The editable install uses this checkout. A PyPI BrainFlow wheel may contain older
DSP behavior and does not substitute for rebuilt `DataHandler`, `BoardController`,
and `MLModule` libraries. The build must include ONNX support for native inference
and export regression checks. See the repository build instructions for platform
compiler prerequisites. The Python ONNX Runtime package validates exports; the
BrainFlow-native ONNX runtime is built and packaged separately.

## Train and reproduce

For the local collection, confirmed by the user as BrainBit board 7:

```sh
python -B src/ml/train/train_classifiers.py --board-id 7 --cache build/reviews/classifier_features.npz
```

Flat `data/relaxed/*.csv` and `data/focused/*.csv` directories require explicit
board metadata. Legacy `data/<class>/<board-id>/*.csv` directories are also
supported. [DATASET.md](DATASET.md) describes the file layout, manifest schema,
grouping rules, and limitations of the current collection.

Useful options:

| Option | Purpose/default |
| --- | --- |
| `--data-dir` | Raw recordings; defaults to `src/ml/train/data`. |
| `--manifest` | Per-recording board, subject, session, or group metadata. |
| `--cache` | NPZ feature cache; defaults to `<work-dir>/features.npz`. |
| `--work-dir` | Search checkpoints and evaluation artifacts; defaults to `build/classifier_training`. |
| `--output-dir` | Final models and reports; defaults to `src/ml/train`. |
| `--jobs` | Parallel search workers; defaults to 6. |
| `--quick` | Reduced candidate search for a faster pipeline check. |
| `--extract-only` | Extract or validate the feature cache without fitting classifiers. |
| `--seed` | Deterministic seed; defaults to 42. |
| `--cv-folds` | Grouped model-selection folds; defaults to 4. |
| `--holdout-folds`, `--holdout-fold` | Holdout partition count/index; defaults to 5 and 0. |
| `--window-seconds` | Canonical window duration; defaults to 8 seconds. |
| `--augmentation-windows` | Extra training durations; defaults to 5 and 12 seconds. Supply no values to disable. |
| `--max-windows-per-recording` | Canonical cap and cap for each augmentation duration; defaults to 128. |
| `--skip-seconds` | Initial samples excluded from each recording; defaults to 10 seconds. |
| `--channel-subsets` | Enable additional training-only channel-subset features. |

For example, use an isolated output directory for a quick smoke run:

```sh
python -B src/ml/train/train_classifiers.py --board-id 7 --quick --jobs 4 --cache build/reviews/classifier_features.npz --work-dir build/classifier_training_quick --output-dir build/classifier_models_quick
```

Canonical windows are nonoverlapping and uniformly distributed through each
recording after the initial skip. Augmented rows retain their recording and group
IDs and are used only in training. Native guard requirements are queried before
extraction. With the current defaults at 250 Hz, an input needs at least 1,172
samples (4.688 seconds), including 330 discarded samples at each edge; the default
five-, eight-, and twelve-second windows satisfy this requirement.

Cache fingerprints include source file sizes/modification times, manifest and
extraction options, resolved board metadata, filter settings, and hashes of the
native DSP binary, Python DSP wrapper, and extractor. Changed inputs or DSP cause
re-extraction. The cache contains NumPy arrays and JSON, with pickle disabled.
Search/checkpoint artifacts also record the data and model configuration needed
to audit a run. Keep raw data and generated local work directories out of commits.

## Evaluation and model selection

The holdout uses complete groups. Model selection uses grouped cross-validation
inside the development partition, with learned preprocessing fitted within each
training fold. By default, a group is the recording's UTC start date across both
labels; manifest subject/session metadata can supply stronger grouping.

Only canonical windows contribute validation and holdout scores. Extra durations
and channel subsets stay within their group's training side. SVM probability
calibration uses group-disjoint folds. Stacking trains its meta-model from grouped
out-of-fold base predictions, rather than predictions on the same rows used to fit
the base estimators. Its training rows are canonical; it does not introduce an
ordinary random-window inner split. Recording-aware fitting limits domination by
long recordings and dense augmentation.

The search covers logistic regression, calibrated SVM, random forest, KNN, MLP,
extra trees, histogram gradient boosting, polynomial logistic regression,
standard gradient boosting, and a grouped stacking ensemble. Read
the generated report for selection scores, held-out results, configuration,
feature provenance, and export parity. A model's final deployment fit uses the
available data after evaluation; its held-out result belongs to the separately
saved evaluation model, not to a fresh test of that final fit.

## Outputs and native integration

The output directory receives ten ONNX models, preserving the strongest
cross-validation configuration from each family:

- `logreg_mindfulness.onnx`
- `svm_mindfulness.onnx`
- `forest_mindfulness.onnx`
- `knn_mindfulness.onnx`
- `mlp_mindfulness.onnx`
- `stacking_mindfulness.onnx`
- `extra_trees_mindfulness.onnx`
- `hist_gradient_boosting_mindfulness.onnx`
- `polynomial_logreg_mindfulness.onnx`
- `gradient_boosting_mindfulness.onnx`

Exports have a fixed float32 input of shape `[1, 5]` and a dense probability output
of shape `[1, 2]`, ordered `[relaxed, focused]`. The exporter verifies ONNX
structure and probability agreement with scikit-learn before replacing a model.
Scaling and learned preprocessing remain inside the ONNX graph.
Calibrated SVMs use double-precision arithmetic internally to avoid cancellation
from quantized support-vector coefficients; their external input/output types
remain float32. Distance-weighted KNN export preserves inverse-distance voting
and the special handling of exactly coincident neighbors.
Export parity compares identical float32 input values, with an absolute
probability tolerance of `2e-5`. It separately reports and bounds changes caused
by rounding the original float64 features (`1e-4`); exact training-row KNN probes
can be sensitive to that input rounding. The saved-model verification also checks
all held-out predictions against the original scikit-learn results.

The selected logistic regression also produces native coefficients with its
affine scaler folded into the five weights and intercept. With the default output
directory it updates `src/ml/generated/mindfulness_model.cpp`; with a custom
output directory it writes `mindfulness_model.cpp` there. Rebuild `MLModule` after
updating the generated native source before using the built-in classifier. ONNX
files remain usable through `ONNX_CLASSIFIER` with the rebuilt native runtime.

`training_report.json` and `training_report.md` are saved to the output and work
directories. The work directory also contains `splits.json`, `selection.json`,
`search.json`, `training.log`, held-out prediction arrays, and
`evaluation_models/*.onnx`. Reports include fold means and standard deviations
for macro F1, precision, recall, accuracy, balanced accuracy, ROC AUC, Brier score,
and log loss. These group-based folds must not be interpreted as thousands of
independent EEG trials.

After rebuilding with the new coefficients, verify the complete saved model set:

```sh
python -B src/ml/train/verify_classifiers.py --output src/ml/train/verification_report.json
```

This checks artifact hashes, all held-out evaluation probabilities against saved
scikit-learn predictions, production models in both ONNX runtimes, and native
mindfulness/restfulness against the generated logistic coefficients. The runtime
verification report is separate from training metrics.

## Tests

After setup and rebuilding the native libraries, run from the repository root:

```sh
python -B -m unittest discover -s src/ml/train -p test_training_data.py
python -B -m unittest discover -s src/ml/train/tests -p "test_*.py"
python -B python_package/examples/tests/ml_binding_regression.py
python -B python_package/examples/tests/ml_export_regression.py
```

The extraction tests check grouping, augmentation isolation, native window
requirements, reporting malformed files, and cache invalidation. Model tests
check grouped fitting/calibration/stacking. Export tests compare scikit-learn,
ONNX, and native predictions and exercise native buffer/output selection.

## Limits of the current data

The local collection has 50 recordings and 25 UTC day groups. All focused
recordings are from six days in 2020; relaxed recordings span 20 days, including
14 files from 2022. Only one day contains both labels. There are no recorded
subject IDs. Consequently, grouped scores can still reflect day, era, or setup
differences and cannot establish performance on unseen people. Report these
limits alongside accuracy and use the 2020-only sensitivity results when
assessing era confounding. Several older recordings also have batched/drifting
timestamps; feature extraction uses the explicit nominal board sampling rate.

Retraining changes the numerical behavior of the built-in classifier and exported
models. Evaluate application thresholds and any downstream smoothing against the
new models and preprocessing before treating their scores as interchangeable
with a previous release.
