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

    # in fact it can be CytonDaisy or Wifi Shield based boards, limit it to only Cyton in demo
    if args.board_id != BoardIds.CYTON_BOARD.value:
        raise ValueError ('wrong board id, should be Cyton')

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
    accel_channels = BoardShim.get_accel_channels (args.board_id)
    analog_channels = BoardShim.get_analog_channels (args.board_id)
    """
    data format for cyton with analog and accel data,
    data[get_other_channels(board_id)[0]] contains cyton end byte
    if end byte is 0xC0 there are accel data in data[get_accel_channels(board_id)[....]] else there are zeros
    if end byte is 0xC1 there are analog data in data[get_analog_channels(board_id)[....]] else there are zeros
    """
    if not other_channels:
        raise ValueError ('no cyton end byte foud')

    print ('end bytes for first 20 packages:')
    print (data[other_channels[0]][0:20])
    
    # we send /2 to enable analog mode so here we will see zeroes
    print ('accel data for first 20 packages:')
    for count, channel in enumerate (accel_channels):
        print (data[channel][0:20])

    # analog data are in int32 format but we return single array fron low level api so it was casted to double wo any changes
    print ('analog data for first 20 packages:')
    for count, channel in enumerate (analog_channels):
        print (data[channel][0:20])


if __name__ == "__main__":
    main ()
