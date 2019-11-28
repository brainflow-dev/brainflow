import argparse
import time
import brainflow
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main ():
    BoardShim.enable_dev_board_logger ()

    # use synthetic board for demo
    params = BrainFlowInputParams ()
    board_id = BoardIds.SYNTHETIC_BOARD.value
    board = BoardShim (board_id, params)
    board.prepare_session ()
    board.start_stream ()
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep (10)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    eeg_channels = BoardShim.get_eeg_channels (board_id)
    # demo for transforms
    for count, channel in enumerate (eeg_channels):
        print ('Original data for channel %d:' % channel)
        print (data[channel])
        # wavelet_coeffs format is[A(J) D(J) D(J-1) ..... D(1)] where J is decomposition level, A - app coeffs, D - detailed coeffs
        # lengths array stores lengths for each block
        wavelet_coeffs, lengths = DataFilter.perform_wavelet_transform (data[channel], 'db5', 3)
        app_coefs = wavelet_coeffs[0: lengths[0]]
        detailed_coeffs_first_block = wavelet_coeffs[lengths[0] : lengths[1]]
        # you can do smth with wavelet coeffs here, for example denoising works via thresholds 
        # for wavelets coefficients
        restored_data = DataFilter.perform_inverse_wavelet_transform ((wavelet_coeffs, lengths), data[channel].shape[0], 'db5', 3)
        print ('Restored data for channel %d:' % channel)
        print (restored_data)


if __name__ == "__main__":
    main ()
