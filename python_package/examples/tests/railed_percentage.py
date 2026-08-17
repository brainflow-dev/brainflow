import numpy as np

from brainflow.data_filter import DataFilter

GAIN = 24
# Same full-scale value get_railed_percentage derives internally, so the expected
# percentages below are readable rather than magic numbers.
SCALER = 4.5 / (2 ** 23 - 1) / GAIN * 1000000.0
MAX_VAL = SCALER * (2 ** 23)

STRAIGHT_LINE = 100.0


def expected_percentage(peak):
    return peak / MAX_VAL * 100.0


def railed(values):
    return DataFilter.get_railed_percentage(np.array(values, dtype=np.float64), GAIN)


def main():
    # A flat line is fully railed regardless of sign. The comparison used to be
    # abs(previous) - current, so a negative flat line produced a large positive
    # difference and was reported as varying signal.
    assert railed([5.0] * 8) == STRAIGHT_LINE
    assert railed([-5.0] * 8) == STRAIGHT_LINE

    # A monotonic ramp is not a flat line. With the old comparison an increasing ramp
    # always gave a negative difference, so it never tripped the check and every ramp
    # was reported as 100 percent railed.
    ramp = [float(i) for i in range(8)]
    assert railed(ramp) != STRAIGHT_LINE
    assert np.isclose(railed(ramp), expected_percentage(7.0))

    # A descending ramp was already handled, so this pins that the fix did not lose it.
    assert np.isclose(railed(ramp[::-1]), expected_percentage(7.0))

    # Fractional amplitudes below 1. The running maximum was an int, so every peak
    # under 1 truncated to 0 and the reported percentage was exactly 0.
    fractional = [0.1, 0.5, 0.25, 0.75, 0.3, 0.6, 0.2, 0.9]
    assert railed(fractional) > 0.0
    assert np.isclose(railed(fractional), expected_percentage(0.9))

    # Signals that cross zero or end at zero. These pin the existing zero-transition
    # semantics rather than changing them: the second half of the straight-line test is
    # abs(current) > 0.00001, so a step *into* zero is ignored while the step back *out*
    # of zero is counted.
    crossing = [-3.0, -1.0, 0.0, 1.0, 3.0, 1.0, 0.0, -1.0]
    assert railed(crossing) != STRAIGHT_LINE
    assert np.isclose(railed(crossing), expected_percentage(3.0))

    # Flat and then a single step down to zero at the end. The step is suppressed because
    # the current sample is zero, so this still reads as a straight line.
    assert railed([5.0, 5.0, 5.0, 0.0]) == STRAIGHT_LINE
    assert railed([-5.0, -5.0, -5.0, 0.0]) == STRAIGHT_LINE
    assert railed([0.0, 0.0, 0.0, 0.0]) == STRAIGHT_LINE

    # A zero in the middle of an otherwise flat line does break it, because the step from
    # zero back up to 5 has a non-zero current sample.
    assert railed([5.0, 5.0, 0.0, 5.0, 5.0]) != STRAIGHT_LINE

    print('railed percentage regression passed')


if __name__ == '__main__':
    main()
