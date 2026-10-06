import numpy as np

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from brainflow.data_filter import DataFilter


def main():
    # Two simultaneous mixtures: rows are channels and columns are samples.
    time = np.arange(1024, dtype=np.float64) / 256.0
    first = np.sin(2.0 * np.pi * 7.0 * time)
    second = np.sin(2.0 * np.pi * 13.0 * time) ** 3
    data = np.vstack((first + 0.3 * second, 0.2 * first + second))
    # Component order and sign are arbitrary.
    w, k, a, s = DataFilter.perform_ica(data, 2)
    fig, axs = plt.subplots(2, 1)
    axs[0].plot(s[0, :])
    axs[0].set_title('Unmixed signal 1')
    axs[1].plot(s[1, :])
    axs[1].set_title('Unmixed signal 2')
    plt.savefig('unmixed_signal.png')


if __name__ == "__main__":
    main()
