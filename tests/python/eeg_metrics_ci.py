import argparse
import time
import brainflow
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds,BrainFlowError
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations, WindowFunctions, DetrendOperations
from brainflow.ml_model import MLModel, BrainFlowMetrics, BrainFlowClassifiers, BrainFlowModelParams
from brainflow.exit_codes import *


def main ():
    BoardShim.enable_board_logger ()
    DataFilter.enable_data_logger ()
    MLModel.enable_ml_logger ()

    parser = argparse.ArgumentParser ()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port
    parser.add_argument ('--timeout', type = int, help  = 'timeout for device discovery or connection', required = False, default = 0)
    parser.add_argument ('--ip-port', type = int, help  = 'ip port', required = False, default = 0)
    parser.add_argument ('--ip-protocol', type = int, help  = 'ip protocol, check IpProtocolType enum', required = False, default = 0)
    parser.add_argument ('--ip-address', type = str, help  = 'ip address', required = False, default = '')
    parser.add_argument ('--serial-port', type = str, help  = 'serial port', required = False, default = '')
    parser.add_argument ('--mac-address', type = str, help  = 'mac address', required = False, default = '')
    parser.add_argument ('--other-info', type = str, help  = 'other info', required = False, default = '')
    parser.add_argument ('--streamer-params', type = str, help  = 'streamer params', required = False, default = '')
    parser.add_argument ('--serial-number', type = str, help  = 'serial number', required = False, default = '')
    parser.add_argument ('--board-id', type = int, help  = 'board id, check docs to get a list of supported boards', required = True)
    parser.add_argument ('--file', type = str, help  = 'file', required = False, default = '')
    parser.add_argument ('--classifier', type = int, help  = 'classifier to use', required = True)
    parser.add_argument ('--metric', type = int, help  = 'metric to use', required = True)
    args = parser.parse_args ()

    params = BrainFlowInputParams ()
    params.ip_port = args.ip_port
    params.serial_port = args.serial_port
    params.mac_address = args.mac_address
    params.other_info = args.other_info
    params.serial_number = args.serial_number
    params.ip_address = args.ip_address
    params.ip_protocol = args.ip_protocol
    params.timeout = args.timeout
    params.file = args.file
    board = BoardShim (args.board_id, params)
<<<<<<< HEAD
    master_board_id = args.board_id
    if ((args.board_id == BoardIds.STREAMING_BOARD.value) or (args.board_id == BoardIds.PLAYBACK_FILE_BOARD.value)):
        try:
            master_board_id = params.other_info
        except:
            raise BrainFlowError ('specify master board id using params.other_info', BrainflowExitCodes.INVALID_ARGUMENTS_ERROR.value)
=======
    master_board_id = board.get_master_board_id()
>>>>>>> 1d24ac118fdc58aac79ca8d42c5330eeae578c42
    sampling_rate = BoardShim.get_sampling_rate (int (master_board_id))
    board.prepare_session ()
    board.start_stream (45000, args.streamer_params)
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep (5) # recommended window size for eeg metric calculation is at least 4 seconds, bigger is better
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    eeg_channels = BoardShim.get_eeg_channels (int(master_board_id))
    bands = DataFilter.get_avg_band_powers (data, eeg_channels, sampling_rate, True)
    feature_vector = np.concatenate ((bands[0], bands[1]))
    print (feature_vector)
    
    model_params = BrainFlowModelParams (args.metric, args.classifier)
    model = MLModel (model_params)
    model.prepare ()
    print ('Model Score: %f' % model.predict (feature_vector))
    model.release ()


if __name__ == "__main__":
    main ()