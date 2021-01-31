# import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, WindowFunctions


def main():
    # demo for data windowing
    window_len = 20

    for window_function in range(4):

        if window_function == WindowFunctions.NO_WINDOW.value:
            print('Window data for NO_WINDOW function:')
            window_data = DataFilter.get_window(WindowFunctions.NO_WINDOW.value, window_len)
        elif window_function == WindowFunctions.HANNING.value:
            print('Window data for HANNING function:')
            window_data = DataFilter.get_window(WindowFunctions.HANNING.value, window_len)
        elif window_function == WindowFunctions.HAMMING.value:
            print('Window data for HAMMING function:')
            window_data = DataFilter.get_window(WindowFunctions.HAMMING.value, window_len)
        else:
            print('Window data for BLACKMAN_HARRIS function:')
            window_data = DataFilter.get_window(WindowFunctions.BLACKMAN_HARRIS.value, window_len)

        print(window_data)


if __name__ == "__main__":
    main()
