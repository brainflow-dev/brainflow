import argparse
import time
import brainflow
import numpy as np

import pandas as pd
import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main():
    BoardShim.enable_dev_board_logger()

    # use synthetic board for demo
    params = BrainFlowInputParams()
    board_id = BoardIds.SYNTHETIC_BOARD.value
    board = BoardShim(board_id, params)
    board.prepare_session()
    board.start_stream()
    BoardShim.log_message(LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep(20)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    # demo how to convert it to pandas DF and plot data
    eeg_channels = BoardShim.get_eeg_channels(board_id)
    df = pd.DataFrame(np.transpose(data))
    plt.figure()
    df[eeg_channels].plot(subplots=True)
    plt.savefig('before_processing.png')

    # demo for denoising, apply different methods to different channels for demo
    for count, channel in enumerate(eeg_channels):
        # first of all you can try simple moving median or moving average with different window size
        if count == 0:
            DataFilter.perform_rolling_filter(data[channel], 3, AggOperations.MEAN.value)
        elif count == 1:
            DataFilter.perform_rolling_filter(data[channel], 3, AggOperations.MEDIAN.value)
        # if methods above dont work for your signal you can try wavelet based denoising
        # feel free to try different functions and decomposition levels
        elif count == 2:
            DataFilter.perform_wavelet_denoising(data[channel], 'db6', 3)
        elif count == 3:
            DataFilter.perform_wavelet_denoising(data[channel], 'bior3.9', 3)
        elif count == 4:
            DataFilter.perform_wavelet_denoising(data[channel], 'sym7', 3)
        elif count == 5:
            # with synthetic board this one looks like the best option, but it depends on many circumstances
            DataFilter.perform_wavelet_denoising(data[channel], 'coif3', 3)

    df = pd.DataFrame(np.transpose(data))
    plt.figure()
    df[eeg_channels].plot(subplots=True)
    plt.savefig('after_processing.png')


if __name__ == "__main__":
    main()
