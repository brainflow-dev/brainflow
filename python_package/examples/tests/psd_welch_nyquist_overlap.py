import threading

import numpy as np

from brainflow.data_filter import DataFilter, WindowOperations
from brainflow.exit_codes import BrainFlowError, BrainFlowExitCodes

HANG_TIMEOUT_SECONDS = 60


def call_with_timeout(fn):
    """Run fn on a worker thread so a non-terminating call fails instead of hanging.

    ctypes releases the GIL around the native call, so the join below still returns while
    the worker spins. Without this, a regression of the overlap guard would stall the whole
    CI job rather than reporting a failure.
    """
    outcome = {}

    def run():
        try:
            fn()
            outcome['returned'] = True
        except BrainFlowError as err:
            outcome['exit_code'] = err.exit_code

    worker = threading.Thread(target=run, daemon=True)
    worker.start()
    worker.join(HANG_TIMEOUT_SECONDS)
    return worker.is_alive(), outcome


def main():
    nfft = 32
    sampling_rate = 128
    segments = 4
    window = WindowOperations.HANNING.value

    rng = np.random.default_rng(7)
    data = rng.standard_normal(nfft * segments)

    # overlap == nfft used to leave the segment cursor stationary, so the averaging loop
    # never advanced and the call spun forever. The valid range is 0 <= overlap < nfft.
    still_running, outcome = call_with_timeout(
        lambda: DataFilter.get_psd_welch(np.copy(data), nfft, nfft, sampling_rate, window)
    )
    assert not still_running, 'get_psd_welch did not terminate with overlap equal to nfft'
    assert outcome.get('exit_code') == BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR.value, outcome

    # With no overlap the segments are disjoint, so each Welch bin is exactly the mean of
    # the per-segment PSD bins and can be checked against get_psd directly.
    ampl, _ = DataFilter.get_psd_welch(np.copy(data), nfft, 0, sampling_rate, window)
    per_segment = np.array(
        [
            DataFilter.get_psd(np.copy(data[i * nfft:(i + 1) * nfft]), sampling_rate, window)[0]
            for i in range(segments)
        ]
    )
    expected = np.mean(per_segment, axis=0)

    assert ampl.shape[0] == nfft // 2 + 1, ampl.shape
    # The averaging loop stopped at nfft / 2, so the final Nyquist bin kept the running sum
    # over all segments instead of the average and read `segments` times too large.
    assert np.allclose(ampl[-1], expected[-1]), (ampl[-1], expected[-1])
    assert np.allclose(ampl, expected), (ampl, expected)

    print('psd welch nyquist and overlap regression passed')


if __name__ == '__main__':
    main()
