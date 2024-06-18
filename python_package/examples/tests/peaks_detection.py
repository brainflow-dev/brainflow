import time

import matplotlib
import numpy as np
import pandas as pd

matplotlib.use('Agg')
import matplotlib.pyplot as plt

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, NoiseTypes, WaveletTypes, DetrendOperations


def main():
    BoardShim.enable_dev_board_logger()
    DataFilter.enable_dev_data_logger()

    params = BrainFlowInputParams()
    #board_id = BoardIds.MUSE_2_BOARD
    board_id = BoardIds.SYNTHETIC_BOARD.value
    board = BoardShim(board_id, params)
    board.prepare_session()
    board.start_stream()
    BoardShim.log_message(LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    eeg_channels = BoardShim.get_eeg_channels(board_id)
    df = pd.DataFrame(np.transpose(data))
    plt.figure()
    df[eeg_channels].plot(subplots=True)
    plt.savefig('before_processing.png')

    df_wavelets = pd.DataFrame(np.transpose(data))
    df_peaks = pd.DataFrame(np.transpose(data))

    for count, channel in enumerate(eeg_channels):
        DataFilter.detrend(data[channel], DetrendOperations.CONSTANT)
        DataFilter.remove_environmental_noise(data[channel], BoardShim.get_sampling_rate(board_id), NoiseTypes.FIFTY_AND_SIXTY)
        # idea is to remove some components with noise and local extremas
        # todo https://github.com/brainflow-dev/brainflow/issues/538
        data[channel] = DataFilter.restore_data_from_wavelet_detailed_coeffs(data[channel], WaveletTypes.DB4, 6, 4)
        df_wavelets[channel] = data[channel]
        # try different params for lag, influence and threshold, more info here https://stackoverflow.com/a/22640362
        # you can also try it wo wavelets
        df_peaks[channel] = DataFilter.detect_peaks_z_score(data[channel], lag=20, influence=0.1, threshold=3.5)

    plt.figure()
    df_wavelets[eeg_channels].plot(subplots=True)
    plt.savefig('wavelets.png')

    plt.figure()
    df_peaks[eeg_channels].plot(subplots=True)
    plt.savefig('peaks.png')


if __name__ == "__main__":
    main()
