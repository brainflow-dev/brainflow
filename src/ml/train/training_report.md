# Mindfulness classifier training

Selection used development group cross-validation. Holdout scores below precede refitting on all recordings.

CV-selected family: **mlp**. The portable native default remains **logreg**.

## Cross-validation

Means across 4 group-disjoint folds; F1 spread is the fold standard deviation, not a confidence interval.

| Family | Macro F1 (mean +/- SD) | Macro precision | Macro recall / balanced accuracy | ROC AUC |
| --- | ---: | ---: | ---: | ---: |
| mlp | 0.9132 +/- 0.0358 | 0.9141 | 0.9170 | 0.9707 |
| svm | 0.9117 +/- 0.0387 | 0.9160 | 0.9138 | 0.9660 |
| polynomial_logreg | 0.9115 +/- 0.0395 | 0.9134 | 0.9143 | 0.9671 |
| hist_gradient_boosting | 0.9108 +/- 0.0359 | 0.9130 | 0.9137 | 0.9694 |
| gradient_boosting | 0.9096 +/- 0.0373 | 0.9121 | 0.9123 | 0.9690 |
| extra_trees | 0.9087 +/- 0.0327 | 0.9097 | 0.9123 | 0.9726 |
| stacking | 0.9077 +/- 0.0267 | 0.9076 | 0.9131 | 0.9707 |
| knn | 0.9065 +/- 0.0336 | 0.9087 | 0.9097 | 0.9687 |
| random_forest | 0.9058 +/- 0.0276 | 0.9046 | 0.9122 | 0.9689 |
| logreg | 0.8992 +/- 0.0290 | 0.8988 | 0.9043 | 0.9610 |

## Frozen holdout

These scores are reported after model selection; they are not used to choose hyperparameters or model families.

| Family | Window macro F1 | Window accuracy | Recording accuracy | Window ROC AUC |
| --- | ---: | ---: | ---: | ---: |
| logreg | 0.8893 | 0.8894 | 1.0000 | 0.9427 |
| svm | 0.9122 | 0.9125 | 1.0000 | 0.9617 |
| random_forest | 0.9012 | 0.9012 | 1.0000 | 0.9556 |
| knn | 0.9092 | 0.9093 | 1.0000 | 0.9579 |
| mlp | 0.9023 | 0.9023 | 1.0000 | 0.9557 |
| stacking | 0.8906 | 0.8906 | 1.0000 | 0.9571 |
| extra_trees | 0.9031 | 0.9031 | 1.0000 | 0.9598 |
| hist_gradient_boosting | 0.8988 | 0.8988 | 1.0000 | 0.9599 |
| polynomial_logreg | 0.9061 | 0.9061 | 1.0000 | 0.9563 |
| gradient_boosting | 0.8903 | 0.8903 | 1.0000 | 0.9551 |
| Historical native weights on current DSP | 0.8527 | 0.8529 | 1.0000 | 0.9419 |

Canonical windows: 4835; augmented training windows: 9910; recordings: 50; groups: 25.

Window metrics give each recording equal total weight. Recording metrics threshold mean window probability. Selection maximizes mean fold window macro F1 at probability 0.5.

## Limitations

- Subject identities are unavailable unless supplied in the manifest; day grouping does not establish subject-independent generalization.
- Collection era and class are confounded in the supplied dataset: focused recordings are from 2020, while many relaxed recordings are from 2022. Models may learn session or hardware differences.
- Only one frozen grouped holdout is evaluated; small numbers of independent groups make scores uncertain. Inspect recording-level results as well as pooled metrics.
- CV selects hyperparameters, augmentation and stacking bases; selected CV scores are optimistic. The untouched holdout evaluates this selection procedure.
- Aborted export-validation attempts may precede this run. Pipeline fixes are not selected using holdout scores; rerunning the same split does not create a new independent validation study.
- Labels describe the recording task and are not independently validated mental-state measurements.
- Holdout scores belong to development-only evaluation models. Final production models are refit on all recordings, including the former holdout.

## Holdout sensitivity within 2020

This descriptive slice uses recording timestamps and is not used to select models.

| Family | Recordings | Macro F1 | Accuracy |
| --- | ---: | ---: | ---: |
| logreg | 5 | 0.8303 | 0.8839 |
| svm | 5 | 0.8836 | 0.9287 |
| random_forest | 5 | 0.8455 | 0.8919 |
| knn | 5 | 0.8661 | 0.9111 |
| mlp | 5 | 0.8482 | 0.8968 |
| stacking | 5 | 0.8279 | 0.8812 |
| extra_trees | 5 | 0.8541 | 0.9028 |
| hist_gradient_boosting | 5 | 0.8382 | 0.8912 |
| polynomial_logreg | 5 | 0.8530 | 0.9013 |
| gradient_boosting | 5 | 0.8211 | 0.8792 |

The JSON report includes splits, candidate scores, extraction provenance, model hashes, export parity, and versions.
Evaluation ONNX files and holdout_predictions.npz in the work directory preserve the evaluation. Production ONNX files are refit on all groups and have no independent test score.
