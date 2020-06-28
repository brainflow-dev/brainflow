import argparse
import time
import brainflow
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations, WindowFunctions


def main ():
    BoardShim.enable_dev_board_logger ()

    # use synthetic board for demo
    params = BrainFlowInputParams ()
    board_id = BoardIds.SYNTHETIC_BOARD.value
    sampling_rate = BoardShim.get_sampling_rate (board_id)
    board = BoardShim (board_id, params)
    board.prepare_session ()
    board.start_stream ()
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep (10)
    data = board.get_current_board_data (DataFilter.get_nearest_power_of_two (sampling_rate))
    board.stop_stream ()
    board.release_session ()

    eeg_channels = BoardShim.get_eeg_channels (board_id)
    for count, channel in enumerate (eeg_channels):
        # optional: subtract mean or detrend
        psd = DataFilter.get_psd (data[channel], sampling_rate, WindowFunctions.BLACKMAN_HARRIS.value)
        band_power_alpha = DataFilter.get_band_power (psd, 7.0, 13.0)
        band_power_beta = DataFilter.get_band_power (psd, 14.0, 30.0)
        print ("alpha/beta:%f", band_power_alpha / band_power_beta)


if __name__ == "__main__":
    main ()
