import numpy as np

from brainflow.data_filter import DataFilter, WindowOperations


def four_term_window(coefficients, window_len):
    a0, a1, a2, a3 = coefficients
    phase = 2.0 * np.pi * np.arange(window_len) / window_len
    return a0 - a1 * np.cos(phase) + a2 * np.cos(2 * phase) - a3 * np.cos(3 * phase)


def main():
    # Four-term Blackman-Harris from F. J. Harris (1978), periodic like the other windows.
    # scipy.signal.windows.blackmanharris(n, sym=False) gives the same values.
    blackman_harris = (0.35875, 0.48829, 0.14128, 0.01168)
    # Nuttall's continuous first derivative window, which BLACKMAN_HARRIS used to return.
    nuttall = (0.355768, 0.487396, 0.144232, 0.012604)

    for window_len in (8, 9, 20, 64, 256):
        window = np.array(DataFilter.get_window(WindowOperations.BLACKMAN_HARRIS.value, window_len))
        expected = four_term_window(blackman_harris, window_len)
        assert np.allclose(window, expected, rtol=0, atol=1e-12), (window_len, window, expected)
        # The two windows differ by up to about 0.006, far above the tolerance above.
        assert not np.allclose(window, four_term_window(nuttall, window_len), rtol=0, atol=1e-4), window_len

    # Blackman-Harris does not reach zero at its first sample, unlike Nuttall's window.
    window = DataFilter.get_window(WindowOperations.BLACKMAN_HARRIS.value, 8)
    assert np.isclose(window[0], 6e-05, rtol=0, atol=1e-12), window[0]

    print('blackman-harris window regression passed')


if __name__ == '__main__':
    main()
