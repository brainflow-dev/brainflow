import numpy as np

from brainflow.data_filter import DataFilter, WaveletExtensionTypes, WaveletTypes

# db15 has a 30 tap filter, so a level 5 decomposition needs a reasonably long signal and
# grows the coefficient array well past the old fixed 2 * (40 + 1) headroom.
DATA_LEN = 1024
DECOMPOSITION_LEVEL = 5


def main():
    rng = np.random.default_rng(3)
    data = rng.standard_normal(DATA_LEN)

    output = DataFilter.perform_wavelet_transform(
        np.copy(data), WaveletTypes.DB15, DECOMPOSITION_LEVEL, WaveletExtensionTypes.SYMMETRIC
    )
    coeffs, lengths = output

    # The binding allocated data_len + 2 * (40 + 1) regardless of decomposition level, but
    # the native side writes sum(lengths) doubles. At db15 level 5 that is 1167 against an
    # allocation of 1106, so the transform wrote past the end of the array. Comparing the
    # returned coefficient count against sum(lengths) is what makes the shortfall visible,
    # because the wrapper slices the buffer it allocated.
    assert coeffs.shape[0] == int(np.sum(lengths)), (coeffs.shape[0], int(np.sum(lengths)))

    # The headroom has to scale with the level, so an under-allocation at any level shows up.
    for level in range(1, DECOMPOSITION_LEVEL + 1):
        level_output = DataFilter.perform_wavelet_transform(
            np.copy(data), WaveletTypes.DB15, level, WaveletExtensionTypes.SYMMETRIC
        )
        level_coeffs, level_lengths = level_output
        assert level_coeffs.shape[0] == int(np.sum(level_lengths)), (
            level,
            level_coeffs.shape[0],
            int(np.sum(level_lengths)),
        )

    # A truncated coefficient array cannot reconstruct the signal, so the round trip is the
    # end-to-end check that nothing was lost.
    restored = DataFilter.perform_inverse_wavelet_transform(
        output, DATA_LEN, WaveletTypes.DB15, DECOMPOSITION_LEVEL, WaveletExtensionTypes.SYMMETRIC
    )
    assert restored.shape[0] == DATA_LEN, restored.shape
    assert np.allclose(restored, data, atol=1e-8), np.max(np.abs(restored - data))

    print('wavelet buffer size regression passed')


if __name__ == '__main__':
    main()
