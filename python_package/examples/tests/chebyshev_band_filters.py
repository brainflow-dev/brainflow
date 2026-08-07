import numpy as np

from brainflow.data_filter import DataFilter, FilterTypes

SAMPLING_RATE = 256
PASS_FREQ = 10.0
STOP_FREQ = 50.0
CENTER_LOW = 5.0
CENTER_HIGH = 15.0
ORDER = 4
RIPPLE = 1.0


def amplitude_at(signal, freq):
    spectrum = np.abs(np.fft.rfft(signal)) * 2 / len(signal)
    return spectrum[int(round(freq * len(signal) / SAMPLING_RATE))]


def main():
    samples = SAMPLING_RATE * 4
    t = np.arange(samples) / SAMPLING_RATE
    # One tone inside the 5 to 15 Hz band and one well outside it.
    signal = np.sin(2 * np.pi * PASS_FREQ * t) + np.sin(2 * np.pi * STOP_FREQ * t)

    assert np.isclose(amplitude_at(signal, PASS_FREQ), 1.0, atol=0.01)
    assert np.isclose(amplitude_at(signal, STOP_FREQ), 1.0, atol=0.01)

    # Chebyshev type 1 band pass and band stop take five design parameters and expect the
    # ripple after the band width. Writing it into slot 3 overwrote the band width, which
    # produced an unusable design and an all-NaN output.
    for name, apply_filter, kept, removed in (
        ('bandpass', DataFilter.perform_bandpass, PASS_FREQ, STOP_FREQ),
        ('bandstop', DataFilter.perform_bandstop, STOP_FREQ, PASS_FREQ),
    ):
        filtered = np.copy(signal)
        apply_filter(
            filtered,
            SAMPLING_RATE,
            CENTER_LOW,
            CENTER_HIGH,
            ORDER,
            FilterTypes.CHEBYSHEV_TYPE_1.value,
            RIPPLE,
        )

        assert np.all(np.isfinite(filtered)), '%s produced non-finite samples' % name
        # The tone the filter is meant to keep survives close to full amplitude.
        assert amplitude_at(filtered, kept) > 0.8, (name, amplitude_at(filtered, kept))
        # The tone it is meant to reject is pushed far down.
        assert amplitude_at(filtered, removed) < 0.05, (name, amplitude_at(filtered, removed))

    print('chebyshev band filter regression passed')


if __name__ == '__main__':
    main()
