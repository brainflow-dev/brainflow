"""Classifier export parity and native ML buffer/selection regression checks.

Run after rebuilding MLModule with BUILD_ONNX=ON, using the local Python package:
    python python_package/examples/tests/ml_export_regression.py
Training checks also require src/ml/train/requirements.txt.
"""

import ctypes
import hashlib
from pathlib import Path
import re
import sys
import tempfile
import unittest
from unittest import mock
import warnings

import numpy as np
import onnx
from onnx import TensorProto, helper
from scipy.special import expit
from sklearn.calibration import CalibratedClassifierCV
from sklearn.ensemble import (ExtraTreesClassifier, GradientBoostingClassifier, HistGradientBoostingClassifier,
                              RandomForestClassifier, StackingClassifier)
from sklearn.linear_model import LogisticRegression
from sklearn.neighbors import KNeighborsClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.pipeline import make_pipeline
from sklearn.preprocessing import PolynomialFeatures, StandardScaler
from sklearn.svm import SVC

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "python_package"))
sys.path.insert(0, str(ROOT / "src/ml/train"))

from brainflow.board_shim import BrainFlowError
from brainflow.ml_model import (
    BrainFlowClassifiers, BrainFlowMetrics, BrainFlowModelParams, MLModel, MLModuleDLL)
from training_export import export_model, extract_native_logistic, write_native_logistic


def samples():
    generator = np.random.default_rng(317)
    data = generator.dirichlet(np.array([2, 3, 4, 2, 1]), size=160)
    labels = (data @ np.array([-2, -1, 3, 1, -2]) > .4).astype(int)
    probes = np.concatenate((data[-20:], np.eye(5), np.full((1, 5), .2)))
    return data[:-20], labels[:-20], probes


def fitted_models():
    data, labels, _ = samples()
    models = {
        "logreg": make_pipeline(StandardScaler(), LogisticRegression(C=.7, random_state=4)),
        "polynomial_logreg": make_pipeline(PolynomialFeatures(2, include_bias=False),
                                           StandardScaler(), LogisticRegression(C=.1, random_state=4)),
        "gradient_boosting": GradientBoostingClassifier(n_estimators=12, max_depth=2, random_state=4),
        "svm": CalibratedClassifierCV(make_pipeline(StandardScaler(), SVC(C=.5, random_state=4)),
                                      ensemble=False, cv=3, method="sigmoid"),
        "forest": RandomForestClassifier(n_estimators=8, max_depth=4, random_state=4),
        "extra_trees": ExtraTreesClassifier(n_estimators=8, max_depth=4, random_state=4),
        "hist_gradient_boosting": HistGradientBoostingClassifier(
            max_iter=15, max_leaf_nodes=7, min_samples_leaf=5, random_state=4),
        "knn": make_pipeline(StandardScaler(), KNeighborsClassifier(n_neighbors=5, weights="distance")),
        "mlp": make_pipeline(StandardScaler(), MLPClassifier(
            hidden_layer_sizes=(6,), activation="logistic", solver="lbfgs",
            max_iter=1000, random_state=4)),
        "stacking": StackingClassifier(estimators=[
            ("linear", LogisticRegression(C=.7, random_state=4)),
            ("trees", RandomForestClassifier(n_estimators=5, max_depth=3, random_state=4)),
            ("neighbors", make_pipeline(StandardScaler(), KNeighborsClassifier(
                n_neighbors=5, weights="distance")))],
            final_estimator=LogisticRegression(C=.3), passthrough=True, cv=2, n_jobs=1),
    }
    return {name: model.fit(data, labels) for name, model in models.items()}


class ExportRegression(unittest.TestCase):
    def test_folded_scaler_probability_parity(self):
        data, labels, probes = samples()
        for center, scale in ((True, True), (False, True), (True, False), (False, False)):
            with self.subTest(center=center, scale=scale):
                model = make_pipeline(StandardScaler(with_mean=center, with_std=scale),
                                      LogisticRegression(C=.7, random_state=4)).fit(data, labels)
                coefficients, intercept = extract_native_logistic(model)
                np.testing.assert_allclose(expit(probes @ coefficients + intercept),
                                           model.predict_proba(probes)[:, 1], atol=1e-14, rtol=0)

    def test_binary_multinomial_probability_parity(self):
        data, labels, probes = samples()
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", FutureWarning)
            model = make_pipeline(StandardScaler(), LogisticRegression(
                C=.7, multi_class="multinomial", random_state=4)).fit(data, labels)
        coefficients, intercept = extract_native_logistic(model)
        np.testing.assert_allclose(expit(probes @ coefficients + intercept),
                                   model.predict_proba(probes)[:, 1], atol=1e-14, rtol=0)

    def test_liblinear_without_intercept_folds_scalar_zero(self):
        data, labels, probes = samples()
        model = make_pipeline(StandardScaler(), LogisticRegression(
            solver="liblinear", fit_intercept=False, random_state=4)).fit(data, labels)
        coefficients, intercept = extract_native_logistic(model)
        np.testing.assert_allclose(expit(probes @ coefficients + intercept),
                                   model.predict_proba(probes)[:, 1], atol=1e-14, rtol=0)

    def test_native_writer_roundtrips_all_parameters(self):
        data, labels, _ = samples()
        model = make_pipeline(StandardScaler(), LogisticRegression(C=.7)).fit(data, labels)
        coefficients, intercept = extract_native_logistic(model)
        self.assertNotEqual(intercept, 0)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "mindfulness_model.cpp"
            metadata = write_native_logistic(model, path)
            content = path.read_text()
            self.assertEqual(metadata["sha256"], hashlib.sha256(path.read_bytes()).hexdigest())
        saved = np.fromstring(re.search(r"\{([^}]+)\}", content).group(1), sep=",")
        saved_intercept = float(re.search(r"intercept = ([^;]+)", content).group(1))
        np.testing.assert_array_equal(saved, coefficients)
        self.assertEqual(saved_intercept, intercept)

    def test_all_model_families_export_with_probability_parity(self):
        _, _, probes = samples()
        with tempfile.TemporaryDirectory() as directory:
            for name, model in fitted_models().items():
                with self.subTest(model=name):
                    path = Path(directory) / (name + ".onnx")
                    metadata = export_model(model, path, probes)
                    self.assertTrue(path.is_file())
                    self.assertEqual(metadata["input_shape"], [1, 5])
                    self.assertEqual(metadata["output_shape"], [1, 2])
                    self.assertLessEqual(metadata["parity"]["max_absolute_error"], 2e-5)
                    onnx.checker.check_model(onnx.load(path))

    def test_calibrated_svm_linear_and_rbf_ensembles_preserve_probabilities(self):
        data, labels, probes = samples()
        with tempfile.TemporaryDirectory() as directory:
            for kernel in ("linear", "rbf"):
                for ensemble in (False, True):
                    with self.subTest(kernel=kernel, ensemble=ensemble):
                        model = CalibratedClassifierCV(make_pipeline(
                            StandardScaler(), SVC(kernel=kernel, C=3.0)),
                            ensemble=ensemble, cv=3).fit(data, labels)
                        metadata = export_model(model, Path(directory) / "calibrated.onnx", probes)
                        self.assertEqual(metadata["exporter"], "double_precision_calibrated_svm")
                        self.assertLess(metadata["parity"]["max_absolute_error"], 1e-6)

    def test_export_failure_preserves_existing_artifact(self):
        data, labels, probes = samples()
        model = LogisticRegression().fit(data, labels)
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "model.onnx"
            path.write_bytes(b"previous validated artifact")
            wrong = np.tile([1., 0.], (len(probes), 1))
            with mock.patch("training_export._runtime_probabilities",
                            return_value=(wrong, "test", "test")):
                with self.assertRaisesRegex(ValueError, "parity failed"):
                    export_model(model, path, probes)
            self.assertEqual(path.read_bytes(), b"previous validated artifact")
            self.assertEqual(len(list(Path(directory).iterdir())), 1)

    def test_export_bytes_are_reproducible(self):
        data, labels, probes = samples()
        model = LogisticRegression().fit(data, labels)
        expected = model.predict_proba(probes)
        with tempfile.TemporaryDirectory() as directory:
            first, second = Path(directory) / "first.onnx", Path(directory) / "second.onnx"
            with mock.patch("training_export._runtime_probabilities",
                            return_value=(expected, "test", "test")):
                export_model(model, first, probes)
                export_model(model, second, probes)
            self.assertEqual(first.read_bytes(), second.read_bytes())

    def test_knn_exact_duplicate_and_near_zero_distance_weights(self):
        data = np.array([[1, 0, 0, 0, 0], [1, 0, 0, 0, 0], [.75, .25, 0, 0, 0],
                         [.5, .5, 0, 0, 0], [.25, .75, 0, 0, 0]], dtype=np.float64)
        labels = np.array([0, 1, 1, 1, 0])
        probes = np.array([[1, 0, 0, 0, 0], [1 - 2**-24, 2**-24, 0, 0, 0],
                           [.875, .125, 0, 0, 0], [.5, .5, 0, 0, 0]], dtype=np.float64)
        with tempfile.TemporaryDirectory() as directory:
            for metric in (1, 2):
                with self.subTest(minkowski_p=metric):
                    model = KNeighborsClassifier(n_neighbors=4, weights="distance", p=metric).fit(
                        data, labels)
                    np.testing.assert_array_equal(model.predict_proba(probes[:1]), [[.5, .5]])
                    metadata = export_model(model, Path(directory) / f"knn_p{metric}.onnx", probes)
                    self.assertEqual(metadata["distance_weight_repairs"], 1)

    def test_rejects_wrong_feature_or_class_contract(self):
        data, labels, probes = samples()
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "model.onnx"
            with self.assertRaisesRegex(ValueError, "classes"):
                export_model(LogisticRegression().fit(data, labels + 1), path, probes)
            with self.assertRaisesRegex(ValueError, "shape"):
                export_model(LogisticRegression().fit(data, labels), path, probes[:, :4])
            with self.assertRaisesRegex(ValueError, "finite"):
                invalid = probes.copy()
                invalid[0, 0] = np.nan
                export_model(LogisticRegression().fit(data, labels), path, invalid)


class NativeMLRegression(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        MLModel.disable_ml_logger()
        cls.directory = tempfile.TemporaryDirectory()
        cls.single = Path(cls.directory.name) / "single.onnx"
        cls.multiple = Path(cls.directory.name) / "multiple.onnx"
        cls.dynamic = Path(cls.directory.name) / "dynamic_output.onnx"
        input_info = helper.make_tensor_value_info("input", TensorProto.FLOAT, [1, 5])

        def save(path, nodes, outputs):
            graph = helper.make_graph(nodes, "regression", [input_info], outputs)
            model = helper.make_model(graph, opset_imports=[helper.make_opsetid("", 11)])
            model.ir_version = 6
            onnx.checker.check_model(model)
            onnx.save(model, path)

        positive = helper.make_tensor_value_info("positive", TensorProto.FLOAT, [1, 5])
        negative = helper.make_tensor_value_info("negative", TensorProto.FLOAT, [1, 5])
        save(cls.single, [helper.make_node("Identity", ["input"], ["positive"])], [positive])
        save(cls.multiple, [helper.make_node("Identity", ["input"], ["positive"]),
                            helper.make_node("Neg", ["input"], ["negative"])], [positive, negative])
        save(cls.dynamic, [helper.make_node("NonZero", ["input"], ["positions"])],
             [helper.make_tensor_value_info("positions", TensorProto.INT64, [2, None])])
        library = MLModuleDLL.get_instance().lib
        cls.raw_predict = staticmethod(ctypes.CFUNCTYPE(
            ctypes.c_int, ctypes.POINTER(ctypes.c_double), ctypes.c_int,
            ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_int), ctypes.c_char_p)(
                ("predict", library)))

    @classmethod
    def tearDownClass(cls):
        cls.directory.cleanup()

    def tearDown(self):
        MLModel.release_all()

    def onnx_model(self, path, output_name="", capacity=16):
        params = BrainFlowModelParams(BrainFlowMetrics.USER_DEFINED, BrainFlowClassifiers.ONNX_CLASSIFIER)
        params.file = str(path)
        params.output_name = output_name
        params.max_array_size = capacity
        return MLModel(params)

    def builtin(self, metric=BrainFlowMetrics.MINDFULNESS, capacity=1):
        params = BrainFlowModelParams(metric, BrainFlowClassifiers.DEFAULT_CLASSIFIER)
        params.max_array_size = capacity
        return MLModel(params)

    def assert_error(self, code, operation):
        with self.assertRaises(BrainFlowError) as failure:
            operation()
        self.assertEqual(failure.exception.exit_code, code)

    def test_builtin_matches_generated_weights_and_restfulness_complement(self):
        content = (ROOT / "src/ml/generated/mindfulness_model.cpp").read_text()
        coefficients = np.fromstring(re.search(r"\{([^}]+)\}", content).group(1), sep=",")
        intercept = float(re.search(r"intercept = ([^;]+)", content).group(1))
        mind = self.builtin()
        rest = self.builtin(BrainFlowMetrics.RESTFULNESS)
        mind.prepare()
        rest.prepare()
        _, _, probes = samples()
        for row in probes:
            expected = expit(row @ coefficients + intercept)
            self.assertAlmostEqual(mind.predict(row)[0], expected, places=13)
            self.assertAlmostEqual(rest.predict(row)[0], 1 - expected, places=13)
        np.testing.assert_array_equal(mind.predict(np.r_[probes[0], 1234.]), mind.predict(probes[0]))

    def test_builtin_rejects_invalid_inputs_and_capacity(self):
        self.assert_error(13, lambda: self.builtin(capacity=0).prepare())
        model = self.builtin()
        model.prepare()
        for data in (np.ones(4), np.array([.2, .2, np.nan, .2, .2]),
                     np.array([.2, .2, np.inf, .2, .2])):
            self.assert_error(13, lambda: model.predict(data))
        values = np.ones(5)
        output = np.zeros(1)
        self.assertEqual(self.raw_predict(
            values.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), 5,
            output.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), None,
            model.serialized_params), 13)

    def test_registry_distinguishes_output_names_and_capacities(self):
        positive = self.onnx_model(self.multiple, "positive", 5)
        negative = self.onnx_model(self.multiple, "negative", 5)
        larger = self.onnx_model(self.multiple, "positive", 6)
        positive.prepare()
        negative.prepare()
        larger.prepare()
        data = np.arange(5, dtype=np.float64)
        np.testing.assert_array_equal(positive.predict(data), data)
        np.testing.assert_array_equal(negative.predict(data), -data)
        np.testing.assert_array_equal(larger.predict(data), data)

    def test_actual_dynamic_output_size_and_empty_output(self):
        model = self.onnx_model(self.dynamic, capacity=8)
        model.prepare()
        np.testing.assert_array_equal(model.predict(np.array([0., .2, 0., .3, 0.])), [0, 0, 1, 3])
        self.assertEqual(model.predict(np.zeros(5)).size, 0)

    def test_output_capacity_rejects_instead_of_truncating(self):
        model = self.onnx_model(self.single, capacity=2)
        model.prepare()
        values = np.arange(5, dtype=np.float64)
        output = np.full(5, 12345., dtype=np.float64)
        count = ctypes.c_int(-1)
        self.assertEqual(self.raw_predict(
            values.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), 5,
            output.ctypes.data_as(ctypes.POINTER(ctypes.c_double)), ctypes.byref(count),
            model.serialized_params), 9)
        self.assertEqual(count.value, 0)
        np.testing.assert_array_equal(output, 12345.)

    def test_explicit_unknown_output_is_rejected_even_for_single_output(self):
        self.assert_error(13, self.onnx_model(self.single, "not_an_output").prepare)

    def test_onnx_rejects_wrong_input_count_nonfinite_and_float_overflow(self):
        model = self.onnx_model(self.single)
        model.prepare()
        for data in (np.ones(4), np.ones(6), np.full(5, np.nan), np.full(5, np.inf),
                     np.full(5, np.finfo(np.float64).max)):
            self.assert_error(13, lambda: model.predict(data))

    def test_exported_models_match_sklearn_in_bundled_runtime(self):
        _, _, probes = samples()
        for name, fitted in fitted_models().items():
            with self.subTest(model=name):
                path = Path(self.directory.name) / (name + ".onnx")
                metadata = export_model(fitted, path, probes)
                model = self.onnx_model(path, metadata["output_name"], capacity=2)
                model.prepare()
                try:
                    probabilities = np.asarray([model.predict(row) for row in probes])
                finally:
                    model.release()
                np.testing.assert_allclose(probabilities, fitted.predict_proba(probes),
                                           atol=2e-5, rtol=0)


if __name__ == "__main__":
    unittest.main()
