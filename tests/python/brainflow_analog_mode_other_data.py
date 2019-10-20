import argparse
import time
import brainflow
import numpy as np

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes

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

    board.config_board ('/2') # enable analog mode only for Cyton Based Boards!

    board.start_stream ()
    time.sleep (10)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    other_channels = BoardShim.get_other_channels (args.board_id)
    print (other_channels)
    for count, channel in enumerate (other_channels):
        print ('count: %d, value %d' % (count, channel))
        # for cyton based board and wifi shiled based boards data format for such rows is
        # count == 0 - cyton end byte
        # count = 1..6 data like described here https://docs.openbci.com/docs/02Cyton/CytonDataFormat#firmware-version-200-fall-2016-to-now-1
        # to get accel data use get_accel_data (board_id) instead get_other_data (board_id)
        # its unsigned char values but since we return an array from low level api they were casted to double wo any modifications
        print (data[channel][0:5])


if __name__ == "__main__":
    main ()
