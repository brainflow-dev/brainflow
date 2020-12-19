import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main ():
    BoardShim.enable_dev_board_logger ()

    # use synthetic board for demo
    params = BrainFlowInputParams ()
    board = BoardShim (BoardIds.SYNTHETIC_BOARD.value, params)
    board.prepare_session ()
    board.start_stream ()
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep (10)
    data = board.get_current_board_data (20) # get 20 latest data points dont remove them from internal buffer
    board.stop_stream ()
    board.release_session ()

    eeg_channels = BoardShim.get_eeg_channels (BoardIds.SYNTHETIC_BOARD.value)
    # demo for data windowing, it demostrates the way window functions modify EEG data
    NO_WINDOW = 0
    HANNING = 1
    HAMMING = 2
    BLACKMAN_HARRIS = 3
    for count, channel in enumerate (eeg_channels):
        print ('Original data for channel %d:' % channel)
        print (data[channel])
        
        if count == NO_WINDOW:
            windowed_data = DataFilter.perform_windowing (data[channel], NO_WINDOW)
        elif count == HANNING:
            windowed_data = DataFilter.perform_windowing (data[channel], HANNING)
        elif count == HAMMING:
            windowed_data = DataFilter.perform_windowing (data[channel], HAMMING)
        else:
            windowed_data = DataFilter.perform_windowing (data[channel], BLACKMAN_HARRIS)

        print ('Windowed data for channel %d:' % channel)
        print (windowed_data)


if __name__ == "__main__":
    main ()
