import time
import numpy as np

from brainflow.board_shim import BoardShim, BoardIds
from brainflow.data_filter import DataFilter


def main():
    board_id = BoardIds.SYNTHETIC_BOARD
    eeg_channels = BoardShim.get_eeg_channels(board_id)
    data = DataFilter.read_file('test_data.csv')
    data = data[:, :10]
    data = np.ascontiguousarray(data)
    w, k, a, s = DataFilter.perform_ica(data, eeg_channels, 2)
    print(w)
    print(k)
    print(a)
    print(s)


if __name__ == "__main__":
    main()
