import argparse
import time
import brainflow
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations, WindowFunctions, DetrendOperations
from brainflow.ml_model import MLModel, BrainFlowMetrics, BrainFlowClassifiers


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
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    eeg_channels = BoardShim.get_eeg_channels (board_id)
    bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, True)
    feature_vector = np.concatenate ((bands[0], bands[1]))
    print(feature_vector)
    
    # calc concentration
    concentration = MLModel (BrainFlowMetrics.CONCENTRATION.value, BrainFlowClassifiers.REGRESSION.value)
    concentration.prepare ()
    print ('Concentration: %f' % concentration.predict (feature_vector))
    concentration.release ()
    
    # calc relaxation
    relaxation = MLModel (BrainFlowMetrics.RELAXATION.value, BrainFlowClassifiers.REGRESSION.value)
    relaxation.prepare ()
    print ('Concentration: %f' % relaxation.predict (feature_vector))
    relaxation.release ()


if __name__ == "__main__":
    main ()
