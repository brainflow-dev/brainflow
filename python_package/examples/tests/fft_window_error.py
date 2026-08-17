import numpy as np

from brainflow.data_filter import DataFilter, DataHandlerDLL, WindowOperations
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes


def main():
    data_len = 64
    data = np.arange(data_len, dtype=np.float64)

    # Call the native entry point directly rather than DataFilter.perform_fft, because the
    # wrapper allocates the output buffers itself and raises before returning them. Filling
    # them with sentinels here is what makes "left untouched" observable.
    original = np.copy(data)
    real = np.full(data_len // 2 + 1, -12345.0)
    imag = np.full(data_len // 2 + 1, -54321.0)

    invalid_window = 999
    res = DataHandlerDLL.get_instance().perform_fft(data, data_len, invalid_window, real, imag)

    assert res == BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value, res
    # perform_fft used to ignore the get_window failure and transform the uninitialized
    # window buffer, so both outputs were written with values derived from garbage.
    assert np.all(real == -12345.0), real
    assert np.all(imag == -54321.0), imag
    assert np.array_equal(data, original), data

    # The same failure through the public wrapper, which turns the code into an exception.
    try:
        DataFilter.perform_fft(np.arange(data_len, dtype=np.float64), invalid_window)
    except BrainFlowError as err:
        assert err.exit_code == BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value, err.exit_code
    else:
        raise AssertionError('an invalid window function must not be accepted')

    # A valid window still round-trips, so the early return did not shadow the normal path.
    restored = DataFilter.perform_ifft(
        DataFilter.perform_fft(np.copy(original), WindowOperations.NO_WINDOW.value)
    )
    assert np.allclose(restored, original), restored

    print('fft window error regression passed')


if __name__ == '__main__':
    main()
