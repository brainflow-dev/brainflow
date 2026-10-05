function binding_validation_regression()
% Run with regenerated DataHandler library and header on the MATLAB path.
    data = sin(2 * pi * 10 * (0:1024) / 256);
    assert(abs(DataFilter.calc_stddev(data) - DataFilter.calc_stddev(data.')) < 1e-12);
    [a, f] = DataFilter.get_psd_welch(data.', 256, 128, 256, int32(WindowOperations.HANNING));
    assert(numel(a) == 129 && numel(f) == 129 && f(end) == 128);
    [coeffs, lengths] = DataFilter.perform_wavelet_transform(data.', int32(WaveletTypes.DB3), 2, int32(WaveletExtensionTypes.SYMMETRIC));
    restored = DataFilter.perform_inverse_wavelet_transform(coeffs.', lengths.', numel(data), int32(WaveletTypes.DB3), 2, int32(WaveletExtensionTypes.SYMMETRIC));
    assert(max(abs(restored - data)) < 1e-10);
    rejects(@() DataFilter.perform_inverse_wavelet_transform(coeffs(1), lengths, numel(data), int32(WaveletTypes.DB3), 2, int32(WaveletExtensionTypes.SYMMETRIC)));
    rejects(@() DataFilter.perform_inverse_wavelet_transform(coeffs, lengths, 64, int32(WaveletTypes.DB3), 2, int32(WaveletExtensionTypes.SYMMETRIC)));
    rejects(@() DataFilter.get_band_power([1, 1], 0, 0, 1));
    rejects(@() DataFilter.get_csp(zeros(4, 2, 20), zeros(3, 1)));
    rejects(@() DataFilter.get_heart_rate([1, 2], 1, 256, 256));
    bands = [2; 8];
    stops = [4; 12];
    matrix = [data; 2 * data];
    [column_bands, ~] = DataFilter.get_custom_band_powers(matrix, bands, stops, [1; 2], 256, false);
    [row_bands, ~] = DataFilter.get_custom_band_powers(matrix, bands.', stops.', [1, 2], 256, false);
    assert(max(abs(column_bands - row_bands)) < 1e-12);
    disp('Binding validation regressions passed');
end

function rejects(action)
    try
        action();
    catch
        return;
    end
    error('Expected invalid input to be rejected');
end
