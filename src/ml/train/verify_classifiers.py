"""Verify saved classifiers against held-out sklearn predictions and native BrainFlow."""
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import sys

import numpy as np
import onnx
import onnxruntime as ort
from scipy.special import expit

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'python_package'))
from brainflow.ml_model import BrainFlowClassifiers, BrainFlowMetrics, BrainFlowModelParams, MLModel


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def native_model(path):
    params = BrainFlowModelParams(BrainFlowMetrics.USER_DEFINED, BrainFlowClassifiers.ONNX_CLASSIFIER)
    params.file = str(Path(path).resolve())
    params.max_array_size = 2
    model = MLModel(params)
    model.prepare()
    return model


def native_predict(model, X):
    try:
        return np.asarray([model.predict(row) for row in X])
    finally:
        model.release()


def ort_predict(path, X):
    options = ort.SessionOptions()
    options.intra_op_num_threads = options.inter_op_num_threads = 1
    session = ort.InferenceSession(str(path), sess_options=options, providers=['CPUExecutionProvider'])
    output = next(value.name for value in session.get_outputs()
                  if value.name in ('probabilities', 'output_probability'))
    name = session.get_inputs()[0].name
    return np.asarray([session.run([output], {name: row.astype(np.float32).reshape(1, 5)})[0][0]
                       for row in X])


def compare(actual, expected, tolerance, description):
    if actual.shape != expected.shape or not np.isfinite(actual).all():
        raise ValueError(f'{description}: invalid prediction shape or nonfinite output')
    error = float(np.max(np.abs(actual - expected)))
    if error > tolerance:
        raise ValueError(f'{description}: max error {error:g} exceeds {tolerance:g}')
    return error


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, default=ROOT / 'src/ml/train/training_report.json')
    parser.add_argument('--work-dir', type=Path, default=ROOT / 'build/classifier_training')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args(argv)
    report = json.loads(args.report.read_text(encoding='utf-8'))
    predictions_path = args.work_dir / 'holdout_predictions.npz'
    if sha256(predictions_path) != report['holdout_predictions_sha256']:
        raise ValueError('Held-out prediction cache does not match the training report')
    with np.load(predictions_path, allow_pickle=False) as cache:
        saved = {name: cache[name] for name in cache.files}
    X = saved['X']
    probes = np.vstack((X[np.linspace(0, len(X)-1, min(128, len(X)), dtype=int)], np.eye(5),
                        np.random.default_rng(42).dirichlet(np.ones(5), 64)))
    MLModel.disable_ml_logger()
    result = dict(training_fingerprint=report['fingerprint'], onnxruntime_version=ort.__version__,
                  brainflow_version=MLModel.get_version(), models={})
    for family, entry in report['models'].items():
        errors = {}
        for stage, directory in (('evaluation', args.work_dir / 'evaluation_models'),
                                 ('production', args.report.parent)):
            metadata = entry[stage + '_export']
            path = directory / Path(metadata['path']).name
            if sha256(path) != metadata['sha256']:
                raise ValueError(f'{family}/{stage}: artifact hash mismatch')
            onnx.checker.check_model(onnx.load(path))
            samples = X if stage == 'evaluation' else probes
            python = ort_predict(path, samples)
            native = native_predict(native_model(path), samples)
            compare(python.sum(axis=1), np.ones(len(samples)), 2e-5, family + ' normalization')
            errors[stage] = dict(probes=len(samples), sha256=metadata['sha256'],
                native_vs_python_max_error=compare(native, python, 2e-5, family + '/' + stage))
            if stage == 'evaluation':
                expected = np.column_stack((1-saved[family], saved[family]))
                errors[stage]['native_vs_sklearn_max_error'] = compare(
                    native, expected, 2e-5, family + '/saved sklearn probabilities')
                errors[stage]['class_disagreements'] = int(np.sum((native[:, 1] >= .5) != (saved[family] >= .5)))
        result['models'][family] = errors
        print(f'{family}: evaluation and production parity passed', flush=True)
    parameters = report['native_logistic']
    native_source = ROOT / 'src/ml/generated/mindfulness_model.cpp'
    if sha256(native_source) != parameters['sha256']:
        raise ValueError('Generated native coefficients do not match the training report')
    probabilities = {}
    for name, metric in [('mindfulness', BrainFlowMetrics.MINDFULNESS), ('restfulness', BrainFlowMetrics.RESTFULNESS)]:
        model = MLModel(BrainFlowModelParams(metric, BrainFlowClassifiers.DEFAULT_CLASSIFIER))
        model.prepare()
        probabilities[name] = native_predict(model, probes)[:, 0]
    expected = expit(probes @ np.asarray(parameters['coefficients']) + parameters['intercept'])
    result['native_default'] = dict(probes=len(probes),
        coefficient_parity_error=compare(probabilities['mindfulness'], expected, 1e-12, 'native logistic'),
        complement_error=compare(probabilities['restfulness'], 1-expected, 1e-12, 'restfulness'))
    logistic = report['models']['logreg']['production_export']
    result['native_default']['onnx_parity_error'] = compare(probabilities['mindfulness'],
        ort_predict(args.report.parent / Path(logistic['path']).name, probes)[:, 1], 2e-5, 'native vs ONNX logistic')
    # This is the dynamically loaded bundled runtime, separate from Python's ORT.
    library_dir = ROOT / 'python_package/brainflow/lib'
    for name in ('onnxruntime_x64.dll', 'libonnxruntime.so', 'libonnxruntime.dylib'):
        path = library_dir / name
        if path.is_file():
            class ApiBase(ctypes.Structure):
                _fields_ = [('get_api', ctypes.c_void_p), ('version', ctypes.c_void_p)]
            library = ctypes.CDLL(str(path))
            library.OrtGetApiBase.restype = ctypes.POINTER(ApiBase)
            version = ctypes.CFUNCTYPE(ctypes.c_char_p)(library.OrtGetApiBase().contents.version)()
            result['bundled_onnxruntime'] = dict(version=version.decode(), sha256=sha256(path))
            break
    result['status'] = 'passed'
    output = args.output or args.work_dir / 'verification_report.json'
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2, sort_keys=True) + '\n', encoding='utf-8')
    print(f'All {len(result["models"])} classifier families and native logistic verified; {output}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
