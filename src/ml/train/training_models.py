"""Small, deployable classifier searches with recording-aware fitting.

All exported estimators consume five band powers. The logistic family uses only
an affine scaler, which can be folded into BrainFlow's native coefficients.
Build each estimator on the exact training subset: calibration and stacking CV
indices must never be reused after an outer split or row selection.
"""

import inspect
import logging
import warnings

import numpy as np
from sklearn import config_context
from sklearn.calibration import CalibratedClassifierCV
from sklearn.ensemble import (ExtraTreesClassifier, GradientBoostingClassifier, HistGradientBoostingClassifier,
                              RandomForestClassifier, StackingClassifier)
from sklearn.linear_model import LogisticRegression
from sklearn.model_selection import StratifiedGroupKFold
from sklearn.neighbors import KNeighborsClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import PolynomialFeatures, StandardScaler
from sklearn.svm import SVC


def grouped_splits(y, groups, n_splits=4, seed=42):
    """Return deterministic group-disjoint folds, with both classes on each side.

    Fewer folds are tried when group composition makes the requested split
    impossible. A failure is explicit; ordinary sample-level CV is never used.
    Every input row appears in exactly one validation fold.
    """
    y, groups = np.asarray(y), np.asarray(groups)
    if y.ndim != 1 or groups.ndim != 1 or len(y) != len(groups) or len(np.unique(y)) != 2:
        raise ValueError('Grouped binary CV requires matching one-dimensional labels and groups')
    available = min(len(np.unique(groups[y == label])) for label in np.unique(y))
    maximum = min(int(n_splits), len(np.unique(groups)), available)
    for count in range(maximum, 1, -1):
        for attempt in range(12):
            cv = StratifiedGroupKFold(count, shuffle=True, random_state=int(seed) + attempt)
            folds = list(cv.split(np.zeros((len(y), 1)), y, groups))
            if all(len(np.unique(y[train])) == len(np.unique(y[test])) == 2 and
                   not np.intersect1d(groups[train], groups[test]).size for train, test in folds):
                return folds
    raise ValueError('Not enough independent groups to put both classes in every CV split')


def candidate_specs(seed=42, quick=False):
    """Return a reproducible, JSON-serializable search of about 12 specs/family."""
    del seed  # Estimator randomness belongs to build_model; the grid itself is fixed.
    configurations = {
        'logreg': [dict(C=c, fit_intercept=True) for c in (0.001, 0.01, 0.1, 1., 10., 100.)
                   for _ in range(2)],
        'svm': ([dict(kernel='linear', C=c, gamma='scale') for c in (0.1, 1., 10.)] +
                [dict(kernel='rbf', C=c, gamma=g) for c in (0.3, 3., 30.) for g in (0.1, 1., 'scale')]),
        'random_forest': [dict(max_depth=depth, min_samples_leaf=leaf,
                               max_features=1.0 if leaf in (3, 30) else 'sqrt')
                          for depth in (3, 6, None) for leaf in (3, 10, 30, 100)],
        'knn': [dict(n_neighbors=k, weights=weight, p=p)
                for k in (7, 21, 51, 101) for weight, p in (('uniform', 2), ('distance', 2), ('distance', 1))],
        'mlp': [dict(hidden_layer_sizes=size, alpha=alpha,
                     activation='relu' if alpha in (0.1, 10.) else 'tanh')
                for size in ([8], [16], [16, 8]) for alpha in (0.01, 0.1, 1., 10.)],
        'hist_gradient_boosting': [dict(max_leaf_nodes=leaves, l2_regularization=penalty)
                                   for leaves in (3, 7, 15) for penalty in (0.1, 1., 10., 100.)],
        'extra_trees': [dict(max_depth=depth, min_samples_leaf=leaf, max_features=1.0)
                        for depth in (3, 6, None) for leaf in (3, 10, 30, 100)],
        'polynomial_logreg': [dict(degree=degree, C=c) for degree in (2, 3)
                              for c in (.001, .01, .1, 1., 10., 100.)],
        'gradient_boosting': [dict(max_depth=depth, min_samples_leaf=leaf, learning_rate=rate)
                              for depth in (1, 2, 3) for leaf in (5, 20) for rate in (.03, .1)],
    }
    quick_indices = {'logreg': (4, 6), 'svm': (1, 6), 'random_forest': (5, 10),
                     'knn': (4, 6), 'mlp': (2, 7), 'hist_gradient_boosting': (5, 6), 'extra_trees': (5, 10),
                     'polynomial_logreg': (2, 8), 'gradient_boosting': (3, 7)}
    specs = [dict(name='logreg_legacy_hyperparameters', family='logreg',
                  params=dict(C=1., fit_intercept=False, solver='liblinear'), augmentation=False)]
    for family, configurations_for_family in configurations.items():
        indices = quick_indices[family] if quick else range(len(configurations_for_family))
        for index in indices:
            augmentation = (index % 2 == 1) if family == 'logreg' else (index % 3 == 2)
            specs.append(dict(name=f'{family}_{index:02d}', family=family,
                              params=configurations_for_family[index], augmentation=augmentation))
    return specs


def _pipeline(classifier):
    return Pipeline([('scaler', StandardScaler()), ('classifier', classifier)])


def build_model(spec, X, y, groups, seed=42):
    """Build an unfitted estimator, keeping all learned transforms in its graph."""
    family, params = spec['family'], dict(spec.get('params', {}))
    if family == 'logreg':
        options = dict(solver='lbfgs', C=1., max_iter=2000, tol=1e-7,
                       fit_intercept=True, random_state=seed, n_jobs=1)
        options.update(params)
        model = _pipeline(LogisticRegression(**options))
    elif family == 'polynomial_logreg':
        degree = params.pop('degree')
        model = Pipeline([('features', PolynomialFeatures(degree=degree, include_bias=False)),
                          ('scaler', StandardScaler()),
                          ('classifier', LogisticRegression(solver='lbfgs', max_iter=3000,
                              tol=1e-7, random_state=seed, n_jobs=1, **params))])
    elif family == 'svm':
        options = dict(kernel='rbf', C=1., gamma='scale', probability=False,
                       cache_size=256, random_state=seed)
        options.update(params)
        options['probability'] = False
        with config_context(enable_metadata_routing=True):
            estimator = Pipeline([
                ('scaler', StandardScaler().set_fit_request(sample_weight=True)),
                ('classifier', SVC(**options).set_fit_request(sample_weight=True))])
        model = CalibratedClassifierCV(estimator, method='sigmoid', ensemble=False, n_jobs=1,
                                      cv=grouped_splits(y, groups, n_splits=3, seed=seed))
    elif family in ('random_forest', 'extra_trees'):
        options = dict(n_estimators=256, n_jobs=1, random_state=seed)
        options.update(params)
        factory = RandomForestClassifier if family == 'random_forest' else ExtraTreesClassifier
        model = factory(**options)
    elif family == 'knn':
        options = dict(n_neighbors=21, weights='distance', p=2, n_jobs=1)
        options.update(params)
        model = _pipeline(KNeighborsClassifier(**options))
    elif family == 'mlp':
        options = dict(hidden_layer_sizes=(16,), solver='lbfgs', activation='tanh',
                       alpha=1., max_iter=3000, max_fun=50000, tol=1e-6,
                       early_stopping=False, random_state=seed)
        options.update(params)
        options['hidden_layer_sizes'] = tuple(options['hidden_layer_sizes'])
        model = _pipeline(MLPClassifier(**options))
    elif family == 'hist_gradient_boosting':
        options = dict(max_iter=150, learning_rate=0.05, min_samples_leaf=20,
                       early_stopping=False, random_state=seed)
        options.update(params)
        model = HistGradientBoostingClassifier(**options)
    elif family == 'gradient_boosting':
        options = dict(n_estimators=200, random_state=seed, n_iter_no_change=None)
        options.update(params)
        model = GradientBoostingClassifier(**options)
    else:
        raise ValueError(f'Unknown classifier family: {family}')
    model._brainflow_seed = int(seed)
    return model


def _recording_weights(y, recording_ids):
    """Equal total mass per recording followed by equal total mass per class."""
    y, recording_ids = np.asarray(y), np.asarray(recording_ids)
    _, inverse, counts = np.unique(recording_ids, return_inverse=True, return_counts=True)
    weights = 1. / counts[inverse]
    for label in np.unique(y):
        selected = y == label
        weights[selected] /= weights[selected].sum()
    return weights * (len(y) / weights.sum())


def _uniform_recording_indices(y, recording_ids, cap=256):
    """Deterministic temporal coverage with equal recording/class contributions.

    These indices are used only in training for estimators without sample weights.
    At most cap rows per recording are retained. Short recordings are repeated
    deterministically to emulate equal recording weights without discarding the
    temporal coverage of every longer recording. These repeats stay in their
    original group and are never used for validation. Each recording has one label.
    """
    y, recording_ids = np.asarray(y), np.asarray(recording_ids)
    records = np.unique(recording_ids)
    rows = [np.flatnonzero(recording_ids == recording) for recording in records]
    labels = []
    for indices in rows:
        labels_for_record = np.unique(y[indices])
        if len(labels_for_record) != 1:
            raise ValueError('A recording must have one class for uniform recording sampling')
        labels.append(labels_for_record[0])
    labels = np.asarray(labels)
    per_class = {label: int(np.sum(labels == label)) for label in np.unique(labels)}
    target = min(int(cap), max(map(len, rows)))
    minority_records = min(per_class.values())
    selected = []
    for indices, label in zip(rows, labels):
        count = max(1, int(target * minority_records / per_class[label]))
        selected.extend(indices[np.linspace(0, len(indices) - 1, count, dtype=int)])
    return np.sort(np.asarray(selected, dtype=int))


def _fit_model(model, X, y, recording_ids):
    """Fit with recording/class-balanced weights, or uniformly capped windows.

    Returns the fitted estimator. Stacking must be built through build_stacking
    so its group metadata can be adjusted when capped training rows are selected.
    """
    X, y, recording_ids = np.asarray(X), np.asarray(y), np.asarray(recording_ids)
    if len(X) != len(y) or len(y) != len(recording_ids) or len(y) == 0:
        raise ValueError('Training arrays must have matching nonzero lengths')
    classifier = model.named_steps['classifier'] if isinstance(model, Pipeline) else model
    unweighted = isinstance(classifier, (KNeighborsClassifier, MLPClassifier, StackingClassifier))
    if unweighted:
        indices = _uniform_recording_indices(y, recording_ids)
        if isinstance(classifier, StackingClassifier):
            if not hasattr(model, '_brainflow_groups') or len(model._brainflow_groups) != len(y):
                raise ValueError('Build stacking on this exact training subset before fitting')
            model.cv = grouped_splits(y[indices], model._brainflow_groups[indices], n_splits=3,
                                      seed=model._brainflow_seed)
            # Neighbors must also fit inside the smallest inner training fold.
            minimum_train = min(len(train) for train, _ in model.cv)
            for _, base in model.estimators:
                inner = base.named_steps['classifier'] if isinstance(base, Pipeline) else base
                if isinstance(inner, KNeighborsClassifier):
                    inner.set_params(n_neighbors=min(inner.n_neighbors, minimum_train))
        elif isinstance(classifier, KNeighborsClassifier):
            classifier.set_params(n_neighbors=min(classifier.n_neighbors, len(indices)))
        model.fit(X[indices], y[indices])
        used_rows = len(indices)
    else:
        weights = _recording_weights(y, recording_ids)
        if isinstance(model, CalibratedClassifierCV):
            with config_context(enable_metadata_routing=True):
                model.fit(X, y, sample_weight=weights)
        elif isinstance(model, Pipeline):
            if 'sample_weight' not in inspect.signature(classifier.fit).parameters:
                raise TypeError('Classifier does not support recording-balanced sample weights')
            model.fit(X, y, classifier__sample_weight=weights, scaler__sample_weight=weights)
        else:
            model.fit(X, y, sample_weight=weights)
        used_rows = len(y)
    model._brainflow_fit_summary = dict(input_rows=len(y), fitted_rows=used_rows,
                                        recordings=len(np.unique(recording_ids)),
                                        weighting='uniform_recording_class_cap' if unweighted else 'equal_recording_class')
    return model


def fit_model(model, X, y, recording_ids):
    """Fit and preserve all fit warnings for candidate and deployment reports."""
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter('always')
        fitted = _fit_model(model, X, y, recording_ids)
    messages = sorted({(warning.category.__name__, str(warning.message)) for warning in caught})
    fitted._brainflow_fit_summary['warnings'] = [dict(category=category, message=message)
                                                for category, message in messages]
    fitted._brainflow_fit_summary['warning_count'] = len(caught)
    fitted._brainflow_fit_summary['convergence_warning'] = any(
        category == 'ConvergenceWarning' for category, _ in messages)
    for category, message in messages:
        logging.warning('%s during model fit: %s', category, message)
    classifier = fitted.named_steps['classifier'] if isinstance(fitted, Pipeline) else fitted
    if hasattr(classifier, 'n_iter_'):
        fitted._brainflow_fit_summary['iterations'] = np.asarray(classifier.n_iter_).tolist()
    return fitted


def build_stacking(specs, X, y, groups, seed=42):
    """Group-cross-fitted stack of LR/RF/KNN/MLP, using canonical training rows.

    SVM is excluded: its calibration group folds would require regeneration for
    each stacking split. The caller must not pass augmented validation rows.
    """
    allowed = {'logreg', 'random_forest', 'knn', 'mlp'}
    chosen, families = [], set()
    for spec in specs:
        family = spec['family']
        if family in allowed and family not in families:
            chosen.append((family, build_model(spec, X, y, groups, seed=seed)))
            families.add(family)
    if len(chosen) < 2:
        raise ValueError('Stacking requires at least two eligible classifier families')
    model = StackingClassifier(
        estimators=chosen, final_estimator=LogisticRegression(C=1., max_iter=2000, random_state=seed, n_jobs=1),
        cv=grouped_splits(y, groups, n_splits=3, seed=seed), stack_method='predict_proba',
        passthrough=False, n_jobs=1)
    model._brainflow_groups = np.asarray(groups).copy()
    model._brainflow_seed = int(seed)
    return model
