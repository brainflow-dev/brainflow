import matplotlib.pyplot as plt
import numpy as np
from scipy.signal import sawtooth

from brainflow.data_filter import DataFilter


def main():
    num_data_points = 1000
    # Source signals
    A = np.sin(np.linspace(0, 50, num_data_points))
    B = -0.35 * sawtooth(np.linspace(0, 86.6, num_data_points))
    C = np.sin(np.linspace(0, 37, num_data_points) + 5)
    # Plot source signals
    fig, axs = plt.subplots(3, 1)
    axs[0].plot(A)
    axs[0].set_title('A')
    axs[1].plot(B)
    axs[1].set_title('B')
    axs[2].plot(C)
    axs[2].set_title('C')
    plt.show()
    # Mixed signals
    M1 = A - 2 * B + 1.125 * C
    M2 = 1.73 * A + 3.41 * B - 1.1 * C
    M3 = C - 0.1 * A + 2.2 * B
    M4 = 0.1 * B + 0.8 * C + 1.1 * A
    # Plot mixed signals
    fig, axs = plt.subplots(1, 1)
    axs.plot(M1)
    axs.plot(M2)
    axs.plot(M3)
    axs.plot(M4)
    axs.set_title('Mixed signals M1, M2, M3, M4')
    plt.show()
    # Concatenate mixed signals
    data = np.ascontiguousarray(np.vstack((M1, M2, M3, M4)))
    relevant_channels = range(4)
    num_components = 3
    w, k, a, s = DataFilter.perform_ica(data, relevant_channels, num_components)
    # Plot unmixed data (s)
    fig, axs = plt.subplots(3, 1)
    axs[0].plot(s[:, 0])
    axs[0].set_title('Unmixed signal 1')
    axs[1].plot(s[:, 1])
    axs[1].set_title('Unmixed signal 2')
    axs[2].plot(s[:, 2])
    axs[2].set_title('Unmixed signal 3')
    plt.show()


if __name__ == "__main__":
    main()
