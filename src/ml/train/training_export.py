"""Export the five-band classifier contract and check executable probability parity."""

import hashlib
import os
from pathlib import Path
import tempfile

import numpy as np
import onnx
from onnx import TensorProto, helper, numpy_helper
from sklearn.calibration import CalibratedClassifierCV
from sklearn.linear_model import LogisticRegression
from sklearn.pipeline import Pipeline
from sklearn.preprocessing import StandardScaler
from sklearn.svm import SVC
from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType


FEATURE_COUNT = 5
ONNX_OPSET = 11
PARITY_ATOL = 2e-5
INPUT_ROUNDING_ATOL = 1e-4


def _classifier(model):
    while isinstance(model, Pipeline):
        model = model.steps[-1][1]
    classes = np.asarray(getattr(model, "classes_", []))
    if classes.shape != (2,) or not np.array_equal(classes, [0, 1]):
        raise ValueError("The classifier must have classes [0, 1]: relaxed, focused")
    return model


def _probes(probe_X):
    data = np.asarray(probe_X, dtype=np.float64)
    if data.ndim != 2 or data.shape[1] != FEATURE_COUNT or data.shape[0] == 0:
        raise ValueError("Parity probes must have shape (n, 5), with n > 0")
    if not np.isfinite(data).all() or np.max(np.abs(data)) > np.finfo(np.float32).max:
        raise ValueError("Parity probes must be finite and representable as float32")
    return np.ascontiguousarray(data)


def _probabilities(values, count):
    values = np.asarray(values, dtype=np.float64)
    if values.shape != (count, 2) or not np.isfinite(values).all():
        raise ValueError("The classifier must return finite probabilities with shape (n, 2)")
    if (np.any(values < -PARITY_ATOL) or np.any(values > 1 + PARITY_ATOL)
            or not np.allclose(values.sum(axis=1), 1, atol=PARITY_ATOL, rtol=0)):
        raise ValueError("Classifier probabilities must be in [0, 1] and sum to one")
    return values


def _conversion_options(model, classifier):
    options = {id(classifier): {"zipmap": False}}
    visited = set()

    def visit(estimator):
        if id(estimator) in visited:
            return
        visited.add(id(estimator))
        if isinstance(estimator, CalibratedClassifierCV):
            for calibrated in estimator.calibrated_classifiers_:
                base = (calibrated.estimator if hasattr(calibrated, "estimator")
                        else calibrated.base_estimator)
                leaf = _classifier(base)
                if isinstance(leaf, SVC):
                    # skl2onnx does not forward this option through the calibrated
                    # estimator's Pipeline. Without it binary SVC scores have the
                    # opposite sign from sklearn.decision_function.
                    options.setdefault(id(leaf), {})["raw_scores"] = True
                visit(base)
        if isinstance(estimator, Pipeline):
            for _, step in estimator.steps:
                visit(step)
        for fitted in getattr(estimator, "estimators_", []):
            visit(fitted)

    visit(model)
    return options


def _repair_distance_weights(model):
    """Correct skl2onnx 1.17's opset-11 KNN sign and zero-neighbor weighting.

    Its TopK selects positive distances, but the following Mul(-1)/Max(1e-6)
    chain turns all inverse-distance weights into equal weights. Match that
    exact pattern, retaining the converter's stable subtraction-based distances.
    The replacement follows sklearn: if any selected neighbors have distance
    zero, only those neighbors vote, with equal weight; otherwise use 1/distance.
    """
    initializers = {item.name: numpy_helper.to_array(item) for item in model.graph.initializer}
    producers = {output: node for node in model.graph.node for output in node.output}
    replacements = {}
    removed = set()
    obsolete_constants = set()

    def scalar(name, expected):
        value = initializers.get(name)
        return value is not None and value.size == 1 and float(value.ravel()[0]) == expected

    for index, node in enumerate(model.graph.node):
        if node.op_type != "Reciprocal":
            continue
        maximum = producers.get(node.input[0])
        if maximum is None or maximum.op_type != "Max" or len(maximum.input) != 2:
            continue
        signed = next((producers.get(name) for name in maximum.input
                       if name not in initializers), None)
        if signed is None or signed.op_type != "Mul" or len(signed.input) != 2:
            continue
        distances = next((name for name in signed.input if not scalar(name, -1.)), None)
        if distances is None or not any(scalar(name, -1.) for name in signed.input):
            continue
        topk = producers.get(distances)
        if (topk is None or topk.op_type != "TopK" or distances != topk.output[0]
                or not any(attr.name == "largest" and attr.i == 0 for attr in topk.attribute)):
            continue

        prefix = f"brainflow_knn_weights_{index}_"
        zero, one = prefix + "zero", prefix + "one"
        model.graph.initializer.extend([
            numpy_helper.from_array(np.array(0, dtype=np.float32), name=zero),
            numpy_helper.from_array(np.array(1, dtype=np.float32), name=one),
        ])
        zero_mask, zero_weights = prefix + "zero_mask", prefix + "zero_weights"
        zero_count, has_zero = prefix + "zero_count", prefix + "has_zero"
        safe_distance, inverse = prefix + "safe_distance", prefix + "inverse"
        replacements[index] = [
            helper.make_node("Equal", [distances, zero], [zero_mask]),
            helper.make_node("Cast", [zero_mask], [zero_weights], to=TensorProto.FLOAT),
            helper.make_node("ReduceSum", [zero_weights], [zero_count], axes=[1], keepdims=1),
            helper.make_node("Greater", [zero_count, zero], [has_zero]),
            helper.make_node("Where", [zero_mask, one, distances], [safe_distance]),
            helper.make_node("Reciprocal", [safe_distance], [inverse]),
            helper.make_node("Where", [has_zero, zero_weights, inverse], list(node.output)),
        ]
        removed.update((signed.output[0], maximum.output[0]))
        obsolete_constants.update(name for name in (*signed.input, *maximum.input)
                                  if name in initializers)

    if replacements:
        rewritten = []
        for index, node in enumerate(model.graph.node):
            if index in replacements:
                rewritten.extend(replacements[index])
            elif not any(output in removed for output in node.output):
                rewritten.append(node)
        del model.graph.node[:]
        model.graph.node.extend(rewritten)
        # Constants may be shared with other estimators in a stacking graph.
        def used_names(graph):
            names = {name for item in graph.node for name in item.input}
            names.update(item.name for item in (*graph.input, *graph.output))
            for item in graph.node:
                for attribute in item.attribute:
                    if attribute.type == onnx.AttributeProto.GRAPH:
                        names.update(used_names(attribute.g))
                    elif attribute.type == onnx.AttributeProto.GRAPHS:
                        for subgraph in attribute.graphs:
                            names.update(used_names(subgraph))
            return names

        used = used_names(model.graph)
        retained = [item for item in model.graph.initializer
                    if item.name not in obsolete_constants or item.name in used]
        del model.graph.initializer[:]
        model.graph.initializer.extend(retained)
    return len(replacements)


def _runtime_probabilities(path, data, output_name):
    """Prefer the training runtime; native BrainFlow is an equivalent fallback."""
    try:
        import onnxruntime as ort
    except ImportError:
        from brainflow.ml_model import (
            BrainFlowClassifiers, BrainFlowMetrics, BrainFlowModelParams, MLModel)

        params = BrainFlowModelParams(
            BrainFlowMetrics.USER_DEFINED, BrainFlowClassifiers.ONNX_CLASSIFIER)
        params.file = str(Path(path).resolve())
        params.output_name = output_name
        params.max_array_size = 2
        runtime = MLModel(params)
        runtime.prepare()
        try:
            values = np.asarray([runtime.predict(row) for row in data], dtype=np.float64)
        finally:
            runtime.release()
        return values, "brainflow_native", MLModel.get_version()

    options = ort.SessionOptions()
    options.intra_op_num_threads = 1
    options.inter_op_num_threads = 1
    session = ort.InferenceSession(
        str(path), sess_options=options, providers=["CPUExecutionProvider"])
    input_name = session.get_inputs()[0].name
    values = [session.run([output_name], {input_name: row.reshape(1, FEATURE_COUNT)})[0][0]
              for row in np.asarray(data, dtype=np.float32)]
    return np.asarray(values, dtype=np.float64), "onnxruntime", ort.__version__


def _calibrated_svm_graph(model):
    """Evaluate binary calibrated SVMs in double precision inside a float API.

    The ai.onnx.ml SVM operator stores its many support vectors/coefficients as
    floats. Cancellation in a real fitted model can exceed the export tolerance.
    Core ONNX arithmetic preserves sklearn's parameters and sigmoid calibration,
    without changing the five-float input or two-float probability output.
    """
    if not isinstance(model, CalibratedClassifierCV) or model.method != "sigmoid":
        return None
    fitted = model.calibrated_classifiers_
    for calibrated in fitted:
        estimator = calibrated.estimator
        leaf = _classifier(estimator)
        if not isinstance(leaf, SVC) or leaf.kernel not in ("linear", "rbf"):
            return None
        if len(calibrated.calibrators) != 1:
            return None
        if isinstance(estimator, Pipeline) and any(
                not isinstance(step, StandardScaler) for _, step in estimator.steps[:-1]):
            return None
    nodes, constants, probabilities = [], [], []

    def constant(name, value):
        constants.append(numpy_helper.from_array(np.asarray(value, dtype=np.float64), name=name))
        return name

    def node(operation, inputs, output, **attributes):
        nodes.append(helper.make_node(operation, inputs, [output], **attributes))
        return output

    raw = node("Cast", ["mindfulness_input"], "double_input", to=TensorProto.DOUBLE)
    for index, calibrated in enumerate(fitted):
        prefix = f"svm{index}_"
        estimator, value = calibrated.estimator, raw
        if isinstance(estimator, Pipeline):
            for number, (_, scaler) in enumerate(estimator.steps[:-1]):
                name = prefix + f"scale{number}_"
                if scaler.with_mean:
                    value = node("Sub", [value, constant(name + "mean", scaler.mean_)], name + "centered")
                if scaler.with_std:
                    value = node("Div", [value, constant(name + "scale", scaler.scale_)], name + "scaled")
        svc = _classifier(estimator)
        if svc.kernel == "linear":
            kernels = node("MatMul", [value, constant(prefix + "support", svc.support_vectors_.T)], prefix + "kernels")
        else:
            difference = node("Sub", [value, constant(prefix + "support", svc.support_vectors_)], prefix + "difference")
            squared = node("Mul", [difference, difference], prefix + "squared")
            distance = node("ReduceSum", [squared], prefix + "distance", axes=[1], keepdims=0)
            exponent = node("Mul", [distance, constant(prefix + "negative_gamma", -svc._gamma)], prefix + "exponent")
            vector = node("Exp", [exponent], prefix + "kernel_vector")
            kernels = node("Unsqueeze", [vector], prefix + "kernels", axes=[0])
        weighted = node("MatMul", [kernels, constant(prefix + "dual", svc.dual_coef_.T)], prefix + "weighted")
        decision = node("Add", [weighted, constant(prefix + "intercept", svc.intercept_[0])], prefix + "decision")
        calibration = calibrated.calibrators[0]
        scaled = node("Mul", [decision, constant(prefix + "negative_a", -calibration.a_)], prefix + "scaled_decision")
        logit = node("Add", [scaled, constant(prefix + "negative_b", -calibration.b_)], prefix + "logit")
        probabilities.append(node("Sigmoid", [logit], prefix + "positive_probability"))
    positive = probabilities[0]
    if len(probabilities) > 1:
        # Older CPU runtimes do not implement Mean for doubles, while Add/Div
        # support them. Preserve sklearn's sum-then-divide ensemble arithmetic.
        for index, probability in enumerate(probabilities[1:], 1):
            positive = node("Add", [positive, probability], f"probability_sum_{index}")
        positive = node("Div", [positive, constant("ensemble_count", len(probabilities))], "mean_probability")
    negative = node("Sub", [constant("one", 1.), positive], "negative_probability")
    dense = node("Concat", [negative, positive], "double_probabilities", axis=1)
    node("Cast", [dense], "probabilities", to=TensorProto.FLOAT)
    node("ArgMax", ["probabilities"], "label", axis=1, keepdims=0)
    graph = helper.make_graph(nodes, "brainflow_calibrated_svm",
        [helper.make_tensor_value_info("mindfulness_input", TensorProto.FLOAT, [1, FEATURE_COUNT])],
        [helper.make_tensor_value_info("label", TensorProto.INT64, [1]),
         helper.make_tensor_value_info("probabilities", TensorProto.FLOAT, [1, 2])], constants)
    return helper.make_model(graph, producer_name="brainflow", ir_version=6,
                             opset_imports=[helper.make_opsetid("", ONNX_OPSET)])


def export_model(model, path, probe_X):
    """Write ONNX only after checking its contract and sklearn probability parity.

    The fixed single-row float input and dense two-column output work with the
    bundled ONNX Runtime 1.11. Probability column 1 means focused/mindfulness;
    column 0 means relaxed/restfulness. A failed export leaves an existing file intact.
    """
    classifier = _classifier(model)
    data = _probes(probe_X)
    original = _probabilities(model.predict_proba(data), len(data))
    # The published API takes float32. Compare execution on exactly those input
    # values, keeping sklearn's fitted transforms/parameters at their precision.
    # KNN probes coincident with float64 training rows are particularly sensitive
    # to the unavoidable input rounding; measure and bound that separately.
    rounded = data.astype(np.float32).astype(np.float64)
    expected = _probabilities(model.predict_proba(rounded), len(data))
    rounding_error = float(np.max(np.abs(expected - original)))
    if rounding_error > INPUT_ROUNDING_ATOL:
        raise ValueError(f"Float32 input rounding changes probabilities by {rounding_error:.8g}")
    exported = _calibrated_svm_graph(model)
    exporter = "double_precision_calibrated_svm" if exported is not None else "skl2onnx"
    if exported is None:
        exported = convert_sklearn(
            model, name="brainflow_mindfulness",
            initial_types=[("mindfulness_input", FloatTensorType([1, FEATURE_COUNT]))],
            target_opset={"": ONNX_OPSET, "ai.onnx.ml": 1},
            options=_conversion_options(model, classifier))
    distance_repairs = _repair_distance_weights(exported)
    operator_sets = sorted(exported.opset_import, key=lambda item: (item.domain, item.version))
    del exported.opset_import[:]
    exported.opset_import.extend(operator_sets)
    if exported.ir_version > 7:
        raise ValueError("Export requires an ONNX IR newer than the bundled runtime supports")
    if any(item.domain in ("", "ai.onnx") and item.version > ONNX_OPSET
           for item in exported.opset_import):
        raise ValueError("Export exceeded the supported ONNX operator set")
    if any(item.domain == "ai.onnx.ml" and item.version > 1 for item in exported.opset_import):
        raise ValueError("Export exceeded the bundled runtime's ai.onnx.ml operator set")
    if any(node.op_type == "ZipMap" for node in exported.graph.node):
        raise ValueError("Native BrainFlow requires dense probabilities, without ZipMap")
    if len(exported.graph.input) != 1:
        raise ValueError("The exported model must have exactly one input")
    input_type = exported.graph.input[0].type.tensor_type
    if (input_type.elem_type != TensorProto.FLOAT
            or [dim.dim_value for dim in input_type.shape.dim] != [1, FEATURE_COUNT]):
        raise ValueError("The exported input must be a float tensor with shape [1, 5]")
    probability_outputs = [value for value in exported.graph.output
                           if value.name in ("probabilities", "output_probability")]
    if len(probability_outputs) != 1:
        raise ValueError("Export must expose one probabilities or output_probability tensor")
    output = probability_outputs[0]
    output_type = output.type.tensor_type
    if (output_type.elem_type != TensorProto.FLOAT or len(output_type.shape.dim) != 2
            or output_type.shape.dim[1].dim_value != 2):
        raise ValueError("The exported probabilities must be a float tensor with shape [1, 2]")
    if output_type.shape.dim[0].HasField("dim_value") and output_type.shape.dim[0].dim_value != 1:
        raise ValueError("The exported probabilities must contain exactly one row")
    # Some sklearn converters leave the batch dimension symbolic even for fixed input.
    output_type.shape.dim[0].dim_value = 1
    onnx.checker.check_model(exported)
    blob = exported.SerializeToString()

    destination = Path(path)
    destination.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary = tempfile.mkstemp(
        prefix=destination.stem + ".", suffix=".onnx", dir=destination.parent)
    try:
        with os.fdopen(descriptor, "wb") as stream:
            stream.write(blob)
        actual, backend, runtime_version = _runtime_probabilities(temporary, data, output.name)
        actual = _probabilities(actual, len(data))
        maximum_error = float(np.max(np.abs(expected - actual)))
        if maximum_error > PARITY_ATOL:
            raise ValueError(
                f"ONNX probability parity failed: max error {maximum_error:.8g} > {PARITY_ATOL}")
        os.replace(temporary, destination)
    finally:
        if os.path.exists(temporary):
            os.unlink(temporary)

    return {
        "path": str(destination),
        "sha256": hashlib.sha256(blob).hexdigest(),
        "bytes": len(blob),
        "input_name": exported.graph.input[0].name,
        "input_shape": [1, FEATURE_COUNT],
        "input_dtype": "float32",
        "output_name": output.name,
        "output_shape": [1, 2],
        "classes": [0, 1],
        "positive_class": 1,
        "exporter": exporter,
        "distance_weight_repairs": distance_repairs,
        "ir_version": exported.ir_version,
        "opsets": {item.domain or "ai.onnx": item.version for item in exported.opset_import},
        "parity": {
            "backend": backend, "runtime_version": runtime_version,
            "probe_count": len(data), "max_absolute_error": maximum_error,
            "absolute_tolerance": PARITY_ATOL,
            "reference_input": "float32 input values evaluated by sklearn in float64",
            "input_rounding_max_error": rounding_error,
            "input_rounding_tolerance": INPUT_ROUNDING_ATOL,
            "original_float64_max_error": float(np.max(np.abs(original - actual))),
        },
    }


def extract_native_logistic(model):
    """Fold fitted StandardScaler steps into a binary logistic model's five weights."""
    classifier = _classifier(model)
    if not isinstance(classifier, LogisticRegression):
        raise ValueError("The native built-in requires a fitted LogisticRegression")
    coefficients = np.asarray(classifier.coef_, dtype=np.float64)
    # liblinear exposes a scalar 0.0 when fit_intercept=False; other solvers
    # expose a length-one array. Both represent the same binary affine model.
    intercept = np.atleast_1d(np.asarray(classifier.intercept_, dtype=np.float64))
    if coefficients.shape != (1, FEATURE_COUNT) or intercept.shape != (1,):
        raise ValueError("The native built-in requires one intercept and five coefficients")
    coefficients = coefficients[0].copy()
    intercept = float(intercept[0])
    if getattr(classifier, "multi_class", None) == "multinomial":
        # sklearn represents a binary softmax with logits (-decision, decision).
        coefficients *= 2.0
        intercept *= 2.0

    def preprocessing_steps(value):
        if isinstance(value, Pipeline):
            steps = []
            for _, step in value.steps[:-1]:
                if isinstance(step, Pipeline):
                    steps.extend(transform for _, transform in step.steps)
                else:
                    steps.append(step)
            return steps + preprocessing_steps(value.steps[-1][1])
        return []

    for step in reversed(preprocessing_steps(model)):
        if step is None or (isinstance(step, str) and step == "passthrough"):
            continue
        if not isinstance(step, StandardScaler):
            raise ValueError("Only StandardScaler preprocessing can be folded into native weights")
        scale = np.asarray(step.scale_, dtype=np.float64) if step.with_std else np.ones(FEATURE_COUNT)
        mean = np.asarray(step.mean_, dtype=np.float64) if step.with_mean else np.zeros(FEATURE_COUNT)
        if (scale.shape != (FEATURE_COUNT,) or mean.shape != (FEATURE_COUNT,)
                or not np.isfinite(scale).all() or np.any(scale <= 0)):
            raise ValueError("The fitted scaler must have five finite, positive scales")
        coefficients /= scale
        intercept -= float(np.dot(coefficients, mean))
    if not np.isfinite(coefficients).all() or not np.isfinite(intercept):
        raise ValueError("The folded native logistic parameters must be finite")
    return coefficients, intercept


def write_native_logistic(model, cpp_path):
    """Serialize the portable default with enough precision to round-trip each double."""
    coefficients, intercept = extract_native_logistic(model)
    text = (
        '#include "mindfulness_model.h"\n\n// clang-format off\n'
        'const double mindfulness_coefficients[5] = {'
        + ','.join(format(value, '.17g') for value in coefficients)
        + '};\n'
        + f'double mindfulness_intercept = {intercept:.17g};\n'
        + '// clang-format on\n')
    path = Path(cpp_path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(text.encode("utf-8"))
    return {"coefficients": coefficients.tolist(), "intercept": intercept,
            "sha256": hashlib.sha256(text.encode("utf-8")).hexdigest()}
