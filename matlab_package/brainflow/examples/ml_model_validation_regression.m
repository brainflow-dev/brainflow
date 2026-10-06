function ml_model_validation_regression(onnx_file, output_name)
% Run with current MLModule library/header on the MATLAB path. Optionally pass
% a binary ONNX classifier with five inputs and two probability outputs.
    data = [0.1, 0.1, 0.3, 0.3, 0.2];
    params = BrainFlowModelParams(int32(0), int32(0));
    params.max_array_size = 2;
    model = MLModel(params);
    model.prepare();
    cleanup = onCleanup(@() model.release());
    expected = model.predict(data);
    assert(numel(expected) == 1 && isfinite(expected) && expected >= 0 && expected <= 1);
    assert(isequal(model.predict(data.'), expected));
    assert(isequal(model.predict(sparse(data)), expected));
    model.input_params.max_array_size = 1;
    model.input_json = '{}';
    assert(isequal(model.predict(data), expected));
    rejects(@() model.predict([]));
    rejects(@() model.predict(ones(2, 3)));
    rejects(@() model.predict([NaN, data(2:end)]));
    rejects(@() model.predict(complex(data, ones(size(data)))));
    clear cleanup;
    for invalid = {0, -1, 1.5, NaN, Inf, double(intmax('int32')) + 1}
        params.max_array_size = invalid{1};
        rejects(@() MLModel(params));
    end
    if nargin >= 1
        if nargin < 2
            output_name = 'probabilities';
        end
        params = BrainFlowModelParams(int32(2), int32(2));
        params.file = onnx_file;
        params.output_name = output_name;
        params.max_array_size = 2;
        model = MLModel(params);
        model.prepare();
        cleanup = onCleanup(@() model.release());
        model.input_params.max_array_size = 1;
        for iteration = 1:20
            result = model.predict(data.');
            assert(numel(result) == 2 && all(isfinite(result)) && abs(sum(result) - 1) < 1e-6);
        end
        clear cleanup;
    end
    disp('ML model binding validation regressions passed');
end

function rejects(action)
    try
        action();
    catch
        return;
    end
    error('Expected invalid input to be rejected');
end
