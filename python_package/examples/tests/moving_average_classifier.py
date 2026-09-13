import os
import sys
import numpy as np

# add python_package to sys.path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

from brainflow.ml_model import MLModel, BrainFlowMetrics, BrainFlowClassifiers, BrainFlowModelParams

V1 = np.array([0.1, 0.2, 0.3, 0.2, 0.2], dtype=np.float64)
V2 = np.array([0.5, 0.1, 0.1, 0.1, 0.2], dtype=np.float64)


def test_baseline_raw_predictions():
    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    model = MLModel(params)
    model.prepare()
    raw_1 = model.predict(V1)[0]
    raw_2 = model.predict(V2)[0]
    model.release()

    assert raw_1 != raw_2
    return raw_1, raw_2


def test_json_explicit_window_len():
    raw_1, raw_2 = test_baseline_raw_predictions()

    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    params.other_info = '{"window_len": 3}'
    model = MLModel(params)
    model.prepare()

    # Step 1: In=v1 -> out1 = raw1
    out1 = model.predict(V1)[0]
    assert np.isclose(out1, raw_1)

    # Step 2: In=v2 -> out2 = (raw1 + raw2) / 2
    out2 = model.predict(V2)[0]
    assert np.isclose(out2, (raw_1 + raw_2) / 2.0)

    # Step 3: In=v2 -> out3 = (raw1 + 2 * raw2) / 3
    out3 = model.predict(V2)[0]
    assert np.isclose(out3, (raw_1 + 2.0 * raw_2) / 3.0)

    # Step 4: In=v2 -> out4 = raw2 (oldest raw1 popped)
    out4 = model.predict(V2)[0]
    assert np.isclose(out4, raw_2)

    model.release()


def test_json_moving_average_with_window_len():
    raw_1, raw_2 = test_baseline_raw_predictions()

    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    params.other_info = '{"moving_average": true, "window_len": 2}'
    model = MLModel(params)
    model.prepare()

    out1 = model.predict(V1)[0]
    assert np.isclose(out1, raw_1)

    out2 = model.predict(V2)[0]
    assert np.isclose(out2, (raw_1 + raw_2) / 2.0)

    model.release()


def test_json_moving_average_integer():
    raw_1, raw_2 = test_baseline_raw_predictions()

    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    params.other_info = '{"moving_average": 3}'
    model = MLModel(params)
    model.prepare()

    out1 = model.predict(V1)[0]
    assert np.isclose(out1, raw_1)

    out2 = model.predict(V2)[0]
    assert np.isclose(out2, (raw_1 + raw_2) / 2.0)

    model.release()


def test_json_default_window():
    raw_1, raw_2 = test_baseline_raw_predictions()

    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    params.other_info = '{"moving_average": true}'
    model = MLModel(params)
    model.prepare()

    for _ in range(5):
        model.predict(V1)

    out6 = model.predict(V2)[0]
    assert np.isclose(out6, (4.0 * raw_1 + raw_2) / 5.0)

    model.release()


def test_json_explicitly_disabled():
    raw_1, raw_2 = test_baseline_raw_predictions()

    # Case 1: '{"moving_average": false}'
    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    params.other_info = '{"moving_average": false}'
    model = MLModel(params)
    model.prepare()
    model.predict(V1)
    out2 = model.predict(V2)[0]
    assert np.isclose(out2, raw_2)
    model.release()

    # Case 2: '{"moving_average": false, "window_len": 3}' (explicit false takes precedence)
    params.other_info = '{"moving_average": false, "window_len": 3}'
    model = MLModel(params)
    model.prepare()
    model.predict(V1)
    out2 = model.predict(V2)[0]
    assert np.isclose(out2, raw_2)
    model.release()


def test_json_window_size_one():
    raw_1, raw_2 = test_baseline_raw_predictions()

    # Case 1: '{"window_len": 1}'
    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    params.other_info = '{"window_len": 1}'
    model = MLModel(params)
    model.prepare()
    model.predict(V1)
    out2 = model.predict(V2)[0]
    assert np.isclose(out2, raw_2)
    model.release()

    # Case 2: '{"moving_average": 1}'
    params.other_info = '{"moving_average": 1}'
    model = MLModel(params)
    model.prepare()
    model.predict(V1)
    out2 = model.predict(V2)[0]
    assert np.isclose(out2, raw_2)
    model.release()


def test_unrelated_json_config_ignored():
    raw_1, raw_2 = test_baseline_raw_predictions()

    params = BrainFlowModelParams(
        BrainFlowMetrics.MINDFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    params.other_info = '{"unrelated_key": "some_value"}'
    model = MLModel(params)
    model.prepare()
    model.predict(V1)
    out2 = model.predict(V2)[0]
    assert np.isclose(out2, raw_2)
    model.release()


def test_non_json_string_ignored():
    raw_1, raw_2 = test_baseline_raw_predictions()

    for config in ["3", "channel_3", "moving_average=3"]:
        params = BrainFlowModelParams(
            BrainFlowMetrics.MINDFULNESS.value,
            BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
        )
        params.other_info = config
        model = MLModel(params)
        model.prepare()
        model.predict(V1)
        out2 = model.predict(V2)[0]
        assert np.isclose(out2, raw_2)
        model.release()


def test_restfulness_moving_average():
    raw_1, raw_2 = test_baseline_raw_predictions()

    rf_params = BrainFlowModelParams(
        BrainFlowMetrics.RESTFULNESS.value,
        BrainFlowClassifiers.DEFAULT_CLASSIFIER.value
    )
    rf_params.other_info = '{"moving_average": true, "window_len": 2}'
    rf_model = MLModel(rf_params)
    rf_model.prepare()

    raw_rf_1 = 1.0 - raw_1
    raw_rf_2 = 1.0 - raw_2

    rf_out1 = rf_model.predict(V1)[0]
    assert np.isclose(rf_out1, raw_rf_1)

    rf_out2 = rf_model.predict(V2)[0]
    assert np.isclose(rf_out2, (raw_rf_1 + raw_rf_2) / 2.0)

    rf_model.release()


def test_vector_output_moving_average():
    import platform
    import shutil
    import subprocess
    import tempfile

    cc = shutil.which('clang') or shutil.which('gcc') or shutil.which('cc')
    if not cc:
        print("C compiler not available, skipping DynLibClassifier vector test.")
        return

    with tempfile.TemporaryDirectory() as tmpdir:
        c_code = """
#if defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT int prepare(void *cls, void *params) { return 0; }
EXPORT int predict(double *data, int data_len, double *output, int *output_len, void *params) {
    output[0] = (data_len > 0) ? data[0] : 1.0;
    output[1] = (data_len > 1) ? data[1] * 2.0 : 2.0;
    output[2] = (data_len > 2) ? data[2] * 3.0 : 3.0;
    *output_len = 3;
    return 0;
}
EXPORT int release(void *params) { return 0; }
"""
        c_file = os.path.join(tmpdir, 'plugin.c')
        ext = '.dylib' if platform.system() == 'Darwin' else ('.dll' if platform.system() == 'Windows' else '.so')
        so_file = os.path.join(tmpdir, 'libplugin' + ext)
        with open(c_file, 'w') as f:
            f.write(c_code)
        cmd = [cc, '-shared', '-fPIC', c_file, '-o', so_file]
        if subprocess.call(cmd) == 0:
            vec_params = BrainFlowModelParams(
                BrainFlowMetrics.USER_DEFINED.value,
                BrainFlowClassifiers.DYN_LIB_CLASSIFIER.value
            )
            vec_params.file = so_file
            vec_params.other_info = '{"window_len": 2}'
            vec_model = MLModel(vec_params)
            vec_model.prepare()

            # Feed sample 1: [10, 10, 10] -> plugin returns [10, 20, 30]
            vec_out1 = vec_model.predict(np.array([10.0, 10.0, 10.0], dtype=np.float64))
            assert len(vec_out1) == 3
            assert np.allclose(vec_out1, [10.0, 20.0, 30.0])

            # Feed sample 2: [20, 20, 20] -> plugin returns [20, 40, 60] -> window avg = [15, 30, 45]
            vec_out2 = vec_model.predict(np.array([20.0, 20.0, 20.0], dtype=np.float64))
            assert len(vec_out2) == 3
            assert np.allclose(vec_out2, [15.0, 30.0, 45.0])

            # Feed sample 3: [20, 20, 20] -> window pops sample 1 -> avg = [20, 40, 60]
            vec_out3 = vec_model.predict(np.array([20.0, 20.0, 20.0], dtype=np.float64))
            assert len(vec_out3) == 3
            assert np.allclose(vec_out3, [20.0, 40.0, 60.0])

            vec_model.release()


if __name__ == '__main__':
    test_baseline_raw_predictions()
    test_json_explicit_window_len()
    test_json_moving_average_with_window_len()
    test_json_moving_average_integer()
    test_json_default_window()
    test_json_explicitly_disabled()
    test_json_window_size_one()
    test_unrelated_json_config_ignored()
    test_non_json_string_ignored()
    test_restfulness_moving_average()
    test_vector_output_moving_average()
    print("All moving average classifier tests passed successfully!")
