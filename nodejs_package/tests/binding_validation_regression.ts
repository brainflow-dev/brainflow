import assert from 'assert';
import {DataFilter, BrainFlowError, WaveletTypes, WaveletExtensionTypes, WindowOperations} from 'brainflow';

assert.throws(() => DataFilter.getBandPower([[1, 1], [0]], 0, 1), BrainFlowError);
assert.throws(() => DataFilter.calcStddev([1, 2], 0, 3), BrainFlowError);
assert.throws(() => DataFilter.calcStddev([1, 2], -1, 2), BrainFlowError);
assert.throws(() => DataFilter.getOxygenLevel([1, 2], [1], 256), BrainFlowError);
assert.throws(() => DataFilter.getHeartRate([1, 2], [1], 256, 256), BrainFlowError);
for (const ragged of [[[1, 2], [3]], [[1], [2, 3]], [[], []]])
{
    assert.throws(() => DataFilter.performIca(ragged, 2, [0, 1]), BrainFlowError);
    assert.throws(() => DataFilter.getCustomBandPowers(ragged, [[0, 1]], [0, 1], 256, false), BrainFlowError);
    assert.throws(() => DataFilter.writeFile(ragged, 'invalid-matrix-must-not-be-written.tsv', 'w'), BrainFlowError);
}
const sparseRows: number[][] = new Array(2);
sparseRows[0] = [1, 2];
assert.throws(() => DataFilter.writeFile(sparseRows, 'invalid-matrix-must-not-be-written.tsv', 'w'), BrainFlowError);
assert.throws(() => DataFilter.writeFile([new Array(2), [1, 2]], 'invalid-matrix-must-not-be-written.tsv', 'w'), BrainFlowError);
for (const channels of [[-1], [2], [0.5], []])
{
    assert.throws(() => DataFilter.performIca([[1, 2], [3, 4]], 2, channels), BrainFlowError);
    assert.throws(() => DataFilter.getCustomBandPowers([[1, 2], [3, 4]], [[0, 1]], channels, 256, false), BrainFlowError);
}
for (const components of [0, 1, 3, 1.5, Number.NaN, Number.POSITIVE_INFINITY])
{
    assert.throws(() => DataFilter.performIca([[1, 2], [3, 4]], components, [0, 1]), BrainFlowError);
}
assert.throws(() => DataFilter.getCustomBandPowers([[1, 2]], [[0]], [0], 256, false), BrainFlowError);
assert.throws(() => DataFilter.performInverseWaveletTransform(
    [[0], [19, 19, 34]], 64, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC), BrainFlowError);
for (const originalLength of [4, 1073741823, 2147483647])
{
    assert.throws(() => DataFilter.performInverseWaveletTransform(
        [[0, 0], [1, 1]], originalLength, WaveletTypes.HAAR, 1, WaveletExtensionTypes.PERIODIC), BrainFlowError);
}

const signal = Array.from({length: 1025}, (_, i) => Math.sin(2 * Math.PI * 10 * i / 256));
const [selectedPowers] = DataFilter.getCustomBandPowers(
    [signal, new Array(signal.length).fill(Number.NaN)], [[8, 12]], [0, 0], 256, false);
assert.strictEqual(selectedPowers[0], 1);
const [amplitudes, frequencies] = DataFilter.getPsdWelch(signal, 256, 128, 256, WindowOperations.HANNING);
assert.strictEqual(amplitudes.length, 129);
assert.strictEqual(frequencies.length, 129);
assert.strictEqual(frequencies[128], 128);
const transformed = DataFilter.performWaveletTransform(signal, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC);
const restored = DataFilter.performInverseWaveletTransform(transformed, signal.length, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC);
assert.ok(restored.every((value, i) => Math.abs(value - signal[i]) < 1e-10));
assert.throws(() => DataFilter.performInverseWaveletTransform(
    transformed, 64, WaveletTypes.DB3, 2, WaveletExtensionTypes.SYMMETRIC), BrainFlowError);
console.log('Binding validation regressions passed');
