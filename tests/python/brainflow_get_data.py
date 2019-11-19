import argparse
import time
import brainflow
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main ():
    parser = argparse.ArgumentParser ()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port
    parser.add_argument ('--ip-port', type = int, help  = 'ip port', required = False, default = 0)
    parser.add_argument ('--ip-protocol', type = int, help  = 'ip protocol, check IpProtocolType enum', required = False, default = 0)
    parser.add_argument ('--ip-address', type = str, help  = 'ip address', required = False, default = '')
    parser.add_argument ('--serial-port', type = str, help  = 'serial port', required = False, default = '')
    parser.add_argument ('--mac-address', type = str, help  = 'mac address', required = False, default = '')
    parser.add_argument ('--other-info', type = str, help  = 'other info', required = False, default = '')
    parser.add_argument ('--board-id', type = int, help  = 'board id, check docs to get a list of supported boards', required = True)
    parser.add_argument ('--log', action = 'store_true')
    args = parser.parse_args ()

    params = BrainFlowInputParams ()
    params.ip_port = args.ip_port
    params.serial_port = args.serial_port
    params.mac_address = args.mac_address
    params.other_info = args.other_info
    params.ip_address = args.ip_address
    params.ip_protocol = args.ip_protocol

    if (args.log):
        BoardShim.enable_dev_board_logger ()
    else:
        BoardShim.disable_board_logger ()

    board = BoardShim (args.board_id, params)
    board.prepare_session ()

    # disable 2nd channel for cyton use real board to check it, emulator ignores commands
    if args.board_id == brainflow.board_shim.BoardIds.CYTON_BOARD.value:
        board.config_board ('x2100000X')

    board.start_stream ()
    time.sleep (10)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    eeg_channels = BoardShim.get_eeg_channels (args.board_id)
    for count, channel in enumerate (eeg_channels):
        # filters work in-place
        if count == 0:
            DataFilter.perform_bandpass (data[channel], BoardShim.get_sampling_rate (args.board_id), 15.0, 6.0, 4, FilterTypes.BESSEL.value, 0)
        elif count == 1:
            DataFilter.perform_bandstop (data[channel], BoardShim.get_sampling_rate (args.board_id), 5.0, 1.0, 3, FilterTypes.BUTTERWORTH.value, 0)
        elif count == 2:
            DataFilter.perform_lowpass (data[channel], BoardShim.get_sampling_rate (args.board_id), 9.0, 5, FilterTypes.CHEBYSHEV_TYPE_1.value, 1)
        elif count == 3:
            DataFilter.perform_highpass (data[channel], BoardShim.get_sampling_rate (args.board_id), 3.0, 4, FilterTypes.BUTTERWORTH.value, 0)
        elif count == 4:
            DataFilter.perform_rolling_filter (data[channel], 3, AggOperations.MEAN.value)
        elif count == 5:
            DataFilter.perform_rolling_filter (data[channel], 3, AggOperations.MEDIAN.value)
        # downsampling returns new numpy array
        elif count == 6:
            downsampled_data = DataFilter.perform_downsampling (data[channel], 2, AggOperations.EACH.value)
            print ("Downsampled data for channel %d:" % channel)
            print (downsampled_data)



if __name__ == "__main__":
    main ()
