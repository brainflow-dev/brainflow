"""Select five-band classifiers on development groups, evaluate a frozen holdout,
then refit production models on all recordings. Reports distinguish both stages.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import importlib.metadata
import json
import logging
from pathlib import Path
import platform
import shutil
import sys
import traceback

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
sys.path.insert(0, str(ROOT / "python_package"))

import numpy as np
from joblib import Parallel, delayed, parallel_config
from scipy.special import expit
from sklearn.metrics import (accuracy_score, balanced_accuracy_score, brier_score_loss,
                             confusion_matrix, f1_score, log_loss, precision_score, recall_score,
                             roc_auc_score)
from threadpoolctl import threadpool_limits
from training_data import extract_dataset
from training_export import export_model, write_native_logistic
from training_models import build_model, build_stacking, candidate_specs, fit_model, grouped_splits

FILENAMES = {"logreg": "logreg_mindfulness.onnx", "svm": "svm_mindfulness.onnx",
             "random_forest": "forest_mindfulness.onnx", "knn": "knn_mindfulness.onnx",
             "mlp": "mlp_mindfulness.onnx", "stacking": "stacking_mindfulness.onnx",
             "extra_trees": "extra_trees_mindfulness.onnx",
             "hist_gradient_boosting": "hist_gradient_boosting_mindfulness.onnx",
             "polynomial_logreg": "polynomial_logreg_mindfulness.onnx",
             "gradient_boosting": "gradient_boosting_mindfulness.onnx"}
LEGACY_COEFFICIENTS = np.array([-1.4060899708538128, 2.597693987367105,
                              -30.96470526503066, 12.04593986553724, 45.773017975354556])


def _json(value):
    return json.dumps(value, sort_keys=True, indent=2, allow_nan=False)


def _write_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(_json(value) + "\n", encoding="utf-8")
    temporary.replace(path)


def _sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _metrics(y, probabilities, weights=None):
    y, probabilities = np.asarray(y), np.asarray(probabilities, dtype=np.float64)
    if (len(y) != len(probabilities) or not np.isfinite(probabilities).all()
            or np.any(probabilities < 0) or np.any(probabilities > 1)):
        raise ValueError("Scoring requires one finite probability in [0, 1] per row")
    predicted = (probabilities >= 0.5).astype(np.int64)
    return {"macro_f1": float(f1_score(y, predicted, labels=[0, 1], average="macro", sample_weight=weights, zero_division=0)),
            "macro_precision": float(precision_score(y, predicted, labels=[0, 1], average="macro", sample_weight=weights, zero_division=0)),
            "macro_recall": float(recall_score(y, predicted, labels=[0, 1], average="macro", sample_weight=weights, zero_division=0)),
            "accuracy": float(accuracy_score(y, predicted, sample_weight=weights)),
            "confusion_matrix": confusion_matrix(y, predicted, labels=[0, 1], sample_weight=weights).tolist(),
            "balanced_accuracy": float(balanced_accuracy_score(y, predicted, sample_weight=weights)),
            "roc_auc": float(roc_auc_score(y, probabilities, sample_weight=weights)),
            "brier": float(brier_score_loss(y, probabilities, sample_weight=weights)),
            "log_loss": float(log_loss(y, np.column_stack((1 - probabilities, probabilities)), labels=[0, 1], sample_weight=weights))}


def score_predictions(y, probabilities, recording_ids):
    """Weight windows equally per recording; separately score mean probabilities."""
    y, probabilities, recording_ids = map(np.asarray, (y, probabilities, recording_ids))
    _, inverse, counts = np.unique(recording_ids, return_inverse=True, return_counts=True)
    record_y, record_probability, records = [], [], []
    for recording in np.unique(recording_ids):
        selected = recording_ids == recording
        labels = np.unique(y[selected])
        if len(labels) != 1:
            raise ValueError("Every recording must have one label")
        probability = float(np.mean(probabilities[selected]))
        record_y.append(int(labels[0]))
        record_probability.append(probability)
        records.append({"recording_id": str(recording), "label": int(labels[0]), "probability": probability,
                        "windows": int(selected.sum())})
    return {"window": _metrics(y, probabilities, 1.0 / counts[inverse]),
            "recording": _metrics(record_y, record_probability),
            "window_count": len(y), "recording_count": len(records), "recordings": records}


def year_sensitivity(dataset, rows, probabilities, year=2020):
    """A descriptive holdout slice using recording timestamps, not group names."""
    records = {record["recording_id"] for record in dataset.metadata.get("recordings", [])
               if str(record.get("recording_day_utc", "")).startswith(f"{year}-")}
    selected = np.isin(dataset.recording_ids[rows], list(records))
    subset = rows[selected]
    if len(np.unique(dataset.y[subset])) != 2:
        return {"status": "unavailable", "year": year, "window_count": len(subset),
                "reason": "The timestamp-defined holdout slice must contain both classes"}
    return {"status": "ok", "year": year, "interpretation": "descriptive slice; never used for selection",
            **score_predictions(dataset.y[subset], np.asarray(probabilities)[selected], dataset.recording_ids[subset])}


def parity_probes(dataset, observed_rows, seed):
    observed = dataset.X[observed_rows[np.linspace(0, len(observed_rows) - 1, min(128, len(observed_rows)), dtype=int)]]
    # Cover feature boundaries and interior compositions as well as observed EEG.
    rng = np.random.default_rng(seed)
    return np.vstack((observed, np.eye(5), np.full((1, 5), .2),
                      rng.dirichlet(np.full(5, .3), size=64), rng.dirichlet(np.ones(5), size=64)))


def freeze_splits(dataset, cv_folds=4, holdout_folds=5, holdout_fold=0, seed=42):
    """Stratify one representative per recording, never thousands of windows."""
    canonical, representative = ~dataset.augmented, []
    for recording in np.unique(dataset.recording_ids):
        all_rows = dataset.recording_ids == recording
        indices = np.flatnonzero(all_rows & canonical)
        if not len(indices):
            raise ValueError(f"Recording {recording} has no canonical feature windows")
        if len(np.unique(dataset.y[all_rows])) != 1 or len(np.unique(dataset.groups[all_rows])) != 1:
            raise ValueError(f"Recording {recording} must have one label and one group")
        representative.append(indices[0])
    representative = np.asarray(representative, dtype=np.int64)
    holdouts = grouped_splits(dataset.y[representative], dataset.groups[representative], n_splits=holdout_folds, seed=seed)
    if holdout_fold < 0 or holdout_fold >= len(holdouts):
        raise ValueError(f"holdout-fold must be in [0, {len(holdouts) - 1}] for these groups")
    development_rep, holdout_rep = (representative[indices] for indices in holdouts[holdout_fold])
    development_groups, holdout_groups = np.unique(dataset.groups[development_rep]), np.unique(dataset.groups[holdout_rep])
    development = np.flatnonzero(np.isin(dataset.groups, development_groups))
    holdout = np.flatnonzero(np.isin(dataset.groups, holdout_groups) & canonical)
    cv = grouped_splits(dataset.y[development_rep], dataset.groups[development_rep], n_splits=cv_folds, seed=seed)
    folds, serialized = [], []
    for train_rep, validation_rep in cv:
        train_groups = np.unique(dataset.groups[development_rep[train_rep]])
        validation_groups = np.unique(dataset.groups[development_rep[validation_rep]])
        train = np.flatnonzero(np.isin(dataset.groups, train_groups))
        validation = np.flatnonzero(np.isin(dataset.groups, validation_groups) & canonical)
        folds.append((train, validation))
        serialized.append({"train_groups": train_groups.tolist(), "validation_groups": validation_groups.tolist(),
                           "train_recordings": np.unique(dataset.recording_ids[train]).tolist(),
                           "validation_recordings": np.unique(dataset.recording_ids[validation]).tolist(),
                           "canonical_validation_windows": len(validation)})
    description = {"seed": seed, "stratification_unit": "one canonical representative per recording",
                   "grouping": "dataset groups; recording UTC day unless overridden by manifest",
                   "requested_holdout_folds": holdout_folds, "actual_holdout_folds": len(holdouts),
                   "holdout_fold": holdout_fold, "requested_cv_folds": cv_folds, "actual_cv_folds": len(cv),
                   "development_groups": development_groups.tolist(), "holdout_groups": holdout_groups.tolist(),
                   "development_recordings": np.unique(dataset.recording_ids[development]).tolist(),
                   "holdout_recordings": np.unique(dataset.recording_ids[holdout]).tolist(),
                   "holdout_canonical_windows": len(holdout), "folds": serialized}
    return development, holdout, folds, description


def _training_indices(indices, augmented, spec):
    return indices if spec["augmentation"] else indices[~augmented[indices]]


def _build(spec, X, y, groups, seed):
    if spec["family"] == "stacking":
        model = build_stacking(spec["params"]["base_specs"], X, y, groups, seed=seed)
        model.set_params(passthrough=spec["params"]["passthrough"])
        model.final_estimator.set_params(C=spec["params"]["C"])
        return model
    return build_model(spec, X, y, groups, seed=seed)


def _probabilities(model, X):
    if not np.array_equal(model.classes_, [0, 1]):
        raise ValueError("Expected classes [0, 1] with focused probability in column 1")
    return np.asarray(model.predict_proba(X), dtype=np.float64)[:, 1]


def evaluate_candidate(spec, X, y, groups, recording_ids, augmented, folds, seed):
    """Worker entry point; folds contain development rows exclusively."""
    try:
        validation_rows, validation_probability, fold_metrics, summaries = [], [], [], []
        with threadpool_limits(limits=1):
            for fold_number, (train_rows, validation) in enumerate(folds):
                train = _training_indices(train_rows, augmented, spec)
                model = _build(spec, X[train], y[train], groups[train], seed + fold_number)
                fit_model(model, X[train], y[train], recording_ids[train])
                probability = _probabilities(model, X[validation])
                fold_metrics.append(score_predictions(y[validation], probability, recording_ids[validation]))
                summaries.append(model._brainflow_fit_summary)
                validation_rows.append(validation)
                validation_probability.append(probability)
        rows, probability = np.concatenate(validation_rows), np.concatenate(validation_probability)
        return {"spec": spec, "status": "ok", "folds": fold_metrics,
                "mean_macro_f1": float(np.mean([fold["window"]["macro_f1"] for fold in fold_metrics])),
                "std_macro_f1": float(np.std([fold["window"]["macro_f1"] for fold in fold_metrics])),
                "pooled": score_predictions(y[rows], probability, recording_ids[rows]), "fit_summaries": summaries}
    except Exception as error:
        return {"spec": spec, "status": "failed", "error": f"{type(error).__name__}: {error}",
                "traceback": traceback.format_exc(limit=5)}


def _spec_key(spec):
    return json.dumps({key: spec[key] for key in ("family", "params", "augmentation")}, sort_keys=True)


def _ranking(result):
    return (-result["mean_macro_f1"], -result["pooled"]["window"]["macro_f1"],
            -result["pooled"]["window"]["roc_auc"], result["spec"]["name"])


def run_search(specs, dataset, folds, args, checkpoint):
    existing = {_spec_key(result["spec"]) for result in checkpoint["results"]}
    pending = []
    for spec in specs:
        if _spec_key(spec) not in existing:
            pending.append(spec)
            existing.add(_spec_key(spec))
    if not pending:
        return
    logging.info("Evaluating %d candidates with %d workers", len(pending), args.jobs)
    with parallel_config(backend="loky", inner_max_num_threads=1):
        results = Parallel(n_jobs=args.jobs, return_as="generator_unordered")(
            delayed(evaluate_candidate)(spec, dataset.X, dataset.y, dataset.groups, dataset.recording_ids,
                                        dataset.augmented, folds, args.seed) for spec in pending)
        for result in results:
            checkpoint["results"].append(result)
            _write_json(args.work_dir / "search.json", checkpoint)
            if result["status"] == "ok":
                logging.info("%s: CV macro F1 %.4f", result["spec"]["name"], result["mean_macro_f1"])
            else:
                logging.error("%s failed: %s", result["spec"]["name"], result["error"])


def _winners(results):
    winners = {}
    for result in sorted((item for item in results if item["status"] == "ok"), key=_ranking):
        winners.setdefault(result["spec"]["family"], result)
    return winners


def _versions():
    versions = {"python": platform.python_version(), "platform": platform.platform()}
    for package in ("numpy", "scipy", "scikit-learn", "joblib", "threadpoolctl", "onnx", "skl2onnx", "onnxruntime"):
        try:
            versions[package] = importlib.metadata.version(package)
        except importlib.metadata.PackageNotFoundError:
            versions[package] = None
    return versions


def _fingerprint(dataset, splits, quick):
    sources = {name: _sha256(HERE / name) for name in
               ("train_classifiers.py", "training_models.py", "training_data.py", "training_export.py")}
    digest = hashlib.sha256()
    for value in (dataset.X, dataset.y, dataset.groups, dataset.recording_ids, dataset.augmented):
        value = np.ascontiguousarray(value)
        digest.update(str((value.dtype, value.shape)).encode())
        digest.update(value.tobytes())
    identity = {"dataset_fingerprint": dataset.metadata.get("fingerprint"), "feature_arrays_sha256": digest.hexdigest(),
                "sources": sources, "splits": splits, "quick": quick, "versions": _versions()}
    return hashlib.sha256(_json(identity).encode()).hexdigest(), identity


def _write_report(args, report):
    for winner in report["models"].values():
        winner["cv_summary"] = {
            metric: {"mean": float(np.mean([fold["window"][metric] for fold in winner["cv"]["folds"]])),
                     "std": float(np.std([fold["window"][metric] for fold in winner["cv"]["folds"]]))}
            for metric in ("macro_f1", "macro_precision", "macro_recall", "accuracy", "balanced_accuracy",
                           "roc_auc", "brier", "log_loss")}
    lines = ["# Mindfulness classifier training", "",
             "Selection used development group cross-validation. Holdout scores below precede refitting on all recordings.", "",
             f"CV-selected family: **{report['selected_global_family']}**. The portable native default remains **logreg**.", "",
             "## Cross-validation", "",
             f"Means across {report['splits']['actual_cv_folds']} group-disjoint folds; F1 spread is the fold standard deviation, not a confidence interval.", "",
             "| Family | Macro F1 (mean +/- SD) | Macro precision | Macro recall / balanced accuracy | ROC AUC |",
             "| --- | ---: | ---: | ---: | ---: |"]
    for family, winner in sorted(report["models"].items(), key=lambda item: -item[1]["cv"]["mean_macro_f1"]):
        cv = winner["cv_summary"]
        lines.append(f"| {family} | {cv['macro_f1']['mean']:.4f} +/- {cv['macro_f1']['std']:.4f} | {cv['macro_precision']['mean']:.4f} | {cv['balanced_accuracy']['mean']:.4f} | {cv['roc_auc']['mean']:.4f} |")
    lines.extend(["", "## Frozen holdout", "",
                  "These scores are reported after model selection; they are not used to choose hyperparameters or model families.", "",
                  "| Family | Window macro F1 | Window accuracy | Recording accuracy | Window ROC AUC |",
                  "| --- | ---: | ---: | ---: | ---: |"])
    for family, winner in report["models"].items():
        holdout = winner["holdout"]
        lines.append(f"| {family} | {holdout['window']['macro_f1']:.4f} | {holdout['window']['accuracy']:.4f} | {holdout['recording']['accuracy']:.4f} | {holdout['window']['roc_auc']:.4f} |")
    baseline = report["historical_native_baseline"]["holdout"]
    lines.extend([f"| Historical native weights on current DSP | {baseline['window']['macro_f1']:.4f} | {baseline['window']['accuracy']:.4f} | {baseline['recording']['accuracy']:.4f} | {baseline['window']['roc_auc']:.4f} |", "",
                  f"Canonical windows: {report['dataset']['canonical_windows']}; augmented training windows: {report['dataset']['augmented_windows']}; recordings: {report['dataset']['recordings']}; groups: {report['dataset']['groups']}.", "",
                  "Window metrics give each recording equal total weight. Recording metrics threshold mean window probability. Selection maximizes mean fold window macro F1 at probability 0.5.", "",
                  "## Limitations", ""])
    lines.extend("- " + limitation for limitation in report["limitations"])
    lines.extend(["", "## Holdout sensitivity within 2020", "",
                  "This descriptive slice uses recording timestamps and is not used to select models.", "",
                  "| Family | Recordings | Macro F1 | Accuracy |",
                  "| --- | ---: | ---: | ---: |"])
    for family, winner in report["models"].items():
        sensitivity = winner["holdout_2020_only"]
        if sensitivity["status"] == "ok":
            lines.append(f"| {family} | {sensitivity['recording_count']} | {sensitivity['window']['macro_f1']:.4f} | {sensitivity['window']['accuracy']:.4f} |")
        else:
            lines.append(f"| {family} | unavailable | - | - |")
    lines.extend(["", "The JSON report includes splits, candidate scores, extraction provenance, model hashes, export parity, and versions.",
                  "Evaluation ONNX files and holdout_predictions.npz in the work directory preserve the evaluation. Production ONNX files are refit on all groups and have no independent test score.", ""])
    for directory in {args.work_dir, args.output_dir}:
        _write_json(directory / "training_report.json", report)
        (directory / "training_report.md").write_text("\n".join(lines), encoding="utf-8")


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--board-id", type=int)
    parser.add_argument("--manifest", type=Path)
    parser.add_argument("--data-dir", type=Path, default=HERE / "data")
    parser.add_argument("--cache", type=Path)
    parser.add_argument("--work-dir", type=Path, default=ROOT / "build" / "classifier_training")
    parser.add_argument("--output-dir", type=Path, default=HERE)
    parser.add_argument("--jobs", type=int, default=6)
    parser.add_argument("--quick", action="store_true", help="Small search for smoke tests")
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--cv-folds", "--cvfolds", type=int, default=4)
    parser.add_argument("--holdout-folds", "--holdoutfolds", type=int, default=5)
    parser.add_argument("--holdout-fold", "--holdoutfold", type=int, default=0)
    parser.add_argument("--extract-only", action="store_true")
    parser.add_argument("--reuse-dataset", action="store_true",
                        help="Deprecated: validated NPZ cache is reused automatically; legacy pickles are unsupported")
    parser.add_argument("--window-seconds", type=float, default=8.0)
    parser.add_argument("--augmentation-windows", type=float, nargs="*", default=[5.0, 12.0])
    parser.add_argument("--max-windows-per-recording", type=int, default=128)
    parser.add_argument("--skip-seconds", type=float, default=10.0)
    parser.add_argument("--channel-subsets", action="store_true", help="Add leave-one-channel-out training augmentation")
    args = parser.parse_args(argv)
    if args.jobs < 1 or args.cv_folds < 2 or args.holdout_folds < 2:
        parser.error("jobs must be positive and CV/holdout folds must be at least two")
    args.work_dir, args.output_dir = args.work_dir.resolve(), args.output_dir.resolve()
    args.cache = (args.cache or args.work_dir / "features.npz").resolve()
    return args


def main(argv=None):
    args = parse_args(argv)
    args.work_dir.mkdir(parents=True, exist_ok=True)
    args.output_dir.mkdir(parents=True, exist_ok=True)
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s",
                        handlers=[logging.StreamHandler(), logging.FileHandler(
                            args.work_dir / "training.log", encoding="utf-8")])
    dataset = extract_dataset(args.data_dir, board_id=args.board_id, manifest=args.manifest,
                              window_seconds=args.window_seconds, augmentation_windows=args.augmentation_windows,
                              max_windows_per_recording=args.max_windows_per_recording, seed=args.seed,
                              cache_path=args.cache, workers=args.jobs, skip_seconds=args.skip_seconds,
                              channel_subsets=True if args.channel_subsets else None)
    logging.info("Features: %d canonical, %d augmented, %d recordings, %d groups", int((~dataset.augmented).sum()),
                 int(dataset.augmented.sum()), len(np.unique(dataset.recording_ids)), len(np.unique(dataset.groups)))
    if args.extract_only:
        return 0
    development, holdout, folds, splits = freeze_splits(dataset, args.cv_folds, args.holdout_folds, args.holdout_fold, args.seed)
    fingerprint, identity = _fingerprint(dataset, splits, args.quick)
    _write_json(args.work_dir / "splits.json", {"fingerprint": fingerprint, **splits})
    checkpoint = {"fingerprint": fingerprint, "identity": identity, "results": []}
    checkpoint_path = args.work_dir / "search.json"
    prior_attempts = []
    if checkpoint_path.is_file():
        try:
            previous = json.loads(checkpoint_path.read_text(encoding="utf-8"))
            if previous.get("fingerprint") == fingerprint:
                checkpoint = previous
                logging.info("Resuming %d checkpointed candidates", len(checkpoint["results"]))
            else:
                logging.info("Ignoring checkpoint with changed data, code, splits or environment")
                prior_attempts.append({"fingerprint": previous.get("fingerprint"),
                                       "completed_candidates": len(previous.get("results", [])),
                                       "reason": "Pipeline or environment changed; prior search is not reused"})
        except (ValueError, KeyError, OSError) as error:
            logging.warning("Ignoring unreadable search checkpoint: %s", error)
    _write_json(checkpoint_path, checkpoint)
    initial_specs = candidate_specs(seed=args.seed, quick=args.quick)
    run_search(initial_specs, dataset, folds, args, checkpoint)
    initial_keys = {_spec_key(spec) for spec in initial_specs}
    alternates = []
    for family in sorted({spec["family"] for spec in initial_specs}):
        successful = sorted((item for item in checkpoint["results"] if item["status"] == "ok"
                             and item["spec"]["family"] == family
                             and _spec_key(item["spec"]) in initial_keys), key=_ranking)
        for result in successful[:3]:
            spec = dict(result["spec"])
            spec["augmentation"] = not spec["augmentation"]
            spec["name"] += "_augmented" if spec["augmentation"] else "_canonical"
            alternates.append(spec)
    run_search(alternates, dataset, folds, args, checkpoint)
    base_winners = _winners(checkpoint["results"])
    expected = set(FILENAMES) - {"stacking"}
    if expected - set(base_winners):
        raise RuntimeError(f"No successful candidate for families: {sorted(expected - set(base_winners))}")
    base_specs = [base_winners[family]["spec"] for family in ("logreg", "random_forest", "knn", "mlp")]
    stacks = [{"name": f"stacking_C{strength:g}_passthrough{int(passthrough)}", "family": "stacking",
               "params": {"C": strength, "passthrough": passthrough, "base_specs": base_specs}, "augmentation": False}
              for strength in ([1.0] if args.quick else [0.1, 1.0, 10.0]) for passthrough in (False, True)]
    run_search(stacks, dataset, folds, args, checkpoint)
    winners = _winners(checkpoint["results"])
    if set(FILENAMES) - set(winners):
        raise RuntimeError("Stacking has no successful group-isolated candidate")
    global_winner = min(winners.values(), key=_ranking)
    selection = {"fingerprint": fingerprint, "selection_metric": "mean fold recording-weighted window macro F1",
                 "selected_global_family": global_winner["spec"]["family"], "selected_global_spec": global_winner["spec"],
                 "families": {family: result["spec"] for family, result in winners.items()}}
    _write_json(args.work_dir / "selection.json", selection)
    logging.info("Selection frozen before holdout evaluation. CV winner: %s", global_winner["spec"]["name"])
    report = {"schema_version": 1, "created_utc": datetime.now(timezone.utc).isoformat(), "fingerprint": fingerprint,
              "identity": identity, "versions": _versions(),
              "prior_attempts": prior_attempts,
              "board_metadata_source": "explicit --board-id" if args.board_id is not None else "manifest",
              "arguments": {key: str(value) if isinstance(value, Path) else value for key, value in vars(args).items()},
              "dataset": {"canonical_windows": int((~dataset.augmented).sum()), "augmented_windows": int(dataset.augmented.sum()),
                          "recordings": len(np.unique(dataset.recording_ids)), "groups": len(np.unique(dataset.groups)),
                          "provenance": dataset.metadata}, "splits": splits, "selection": selection,
              "selected_global_family": selection["selected_global_family"], "native_default_family": "logreg",
              "search_results": checkpoint["results"], "models": {}, "limitations": [
                  "Subject identities are unavailable unless supplied in the manifest; day grouping does not establish subject-independent generalization.",
                  "Collection era and class are confounded in the supplied dataset: focused recordings are from 2020, while many relaxed recordings are from 2022. Models may learn session or hardware differences.",
                  "Only one frozen grouped holdout is evaluated; small numbers of independent groups make scores uncertain. Inspect recording-level results as well as pooled metrics.",
                  "CV selects hyperparameters, augmentation and stacking bases; selected CV scores are optimistic. The untouched holdout evaluates this selection procedure.",
                  "Aborted export-validation attempts may precede this run. Pipeline fixes are not selected using holdout scores; rerunning the same split does not create a new independent validation study.",
                  "Labels describe the recording task and are not independently validated mental-state measurements.",
                  "Holdout scores belong to development-only evaluation models. Final production models are refit on all recordings, including the former holdout."]}
    baseline_probability = expit(dataset.X[holdout] @ LEGACY_COEFFICIENTS)
    report["historical_native_baseline"] = {"coefficients": LEGACY_COEFFICIENTS.tolist(), "intercept": 0.0,
                                          "feature_source": "current DSP, not historical extraction",
                                          "holdout": score_predictions(dataset.y[holdout], baseline_probability, dataset.recording_ids[holdout]),
                                          "holdout_2020_only": year_sensitivity(dataset, holdout, baseline_probability)}
    predictions = {"X": dataset.X[holdout], "y": dataset.y[holdout], "recording_ids": dataset.recording_ids[holdout],
                   "groups": dataset.groups[holdout], "historical_native": baseline_probability}
    probes = parity_probes(dataset, development[~dataset.augmented[development]], args.seed)
    staging = args.work_dir / "release_staging" / fingerprint
    staging.mkdir(parents=True, exist_ok=True)
    publications = []
    all_rows, final_logistic = np.arange(len(dataset.y)), None
    with threadpool_limits(limits=1):
        for family in FILENAMES:
            result, spec = winners[family], winners[family]["spec"]
            logging.info("Evaluating frozen %s winner", family)
            train = _training_indices(development, dataset.augmented, spec)
            model = _build(spec, dataset.X[train], dataset.y[train], dataset.groups[train], args.seed)
            fit_model(model, dataset.X[train], dataset.y[train], dataset.recording_ids[train])
            probability = _probabilities(model, dataset.X[holdout])
            predictions[family] = probability
            staged_evaluation = staging / "evaluation_models" / FILENAMES[family]
            evaluation_target = args.work_dir / "evaluation_models" / FILENAMES[family]
            evaluation = export_model(model, staged_evaluation, probes)
            evaluation["path"] = str(evaluation_target)
            publications.append((staged_evaluation, evaluation_target))
            entry = {"spec": spec, "cv": result, "holdout": score_predictions(dataset.y[holdout], probability, dataset.recording_ids[holdout]),
                     "holdout_2020_only": year_sensitivity(dataset, holdout, probability),
                     "evaluation_export": evaluation, "evaluation_fit": model._brainflow_fit_summary}
            logging.info("Refitting %s on all recording groups", family)
            train = _training_indices(all_rows, dataset.augmented, spec)
            final_model = _build(spec, dataset.X[train], dataset.y[train], dataset.groups[train], args.seed)
            fit_model(final_model, dataset.X[train], dataset.y[train], dataset.recording_ids[train])
            staged_production = staging / "production_models" / FILENAMES[family]
            production_target = args.output_dir / FILENAMES[family]
            entry["production_export"] = export_model(final_model, staged_production, probes)
            entry["production_export"]["path"] = str(production_target)
            publications.append((staged_production, production_target))
            entry["production_fit"] = final_model._brainflow_fit_summary
            report["models"][family] = entry
            if family == "logreg":
                final_logistic = final_model
            _write_json(args.work_dir / "training_report.partial.json", report)
            with (args.work_dir / "holdout_predictions.npz.tmp").open("wb") as stream:
                np.savez_compressed(stream, **predictions)
            (args.work_dir / "holdout_predictions.npz.tmp").replace(args.work_dir / "holdout_predictions.npz")
    native_path = (HERE.parent / "generated" / "mindfulness_model.cpp" if args.output_dir == HERE
                   else args.output_dir / "mindfulness_model.cpp")
    staged_native = staging / "mindfulness_model.cpp"
    native = write_native_logistic(final_logistic, staged_native)
    native_probability = expit(probes @ np.asarray(native["coefficients"]) + native["intercept"])
    native["parity_max_absolute_error"] = float(np.max(np.abs(native_probability - _probabilities(final_logistic, probes))))
    if native["parity_max_absolute_error"] > 1e-10:
        raise RuntimeError("Folded native logistic coefficients failed probability parity")
    report["native_logistic"] = {"path": str(native_path), **native}
    publications.append((staged_native, native_path))
    # Only publish after every evaluation/production graph and native parameters
    # pass validation. Each replacement is atomic, including across source drives.
    for source, target in publications:
        target.parent.mkdir(parents=True, exist_ok=True)
        temporary = target.with_suffix(target.suffix + ".tmp")
        shutil.copyfile(source, temporary)
        temporary.replace(target)
    report["publication"] = {"validated_before_publish": True, "artifact_count": len(publications),
                             "staging_directory": str(staging), "atomicity": "per-file replacement"}
    report["holdout_predictions_sha256"] = _sha256(args.work_dir / "holdout_predictions.npz")
    report["completed_utc"] = datetime.now(timezone.utc).isoformat()
    _write_report(args, report)
    logging.info("Training complete: %s", args.output_dir / "training_report.md")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception:
        logging.exception("Training failed; validated search results remain in the work directory")
        raise
