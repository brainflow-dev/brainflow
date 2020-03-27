import argparse
import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main ():
    parser = argparse.ArgumentParser ()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port
    parser.add_argument ('--timeout', type = int, help  = 'timeout for device discovery or connection', required = False, default = 0)
    parser.add_argument ('--ip-port', type = int, help  = 'ip port', required = False, default = 0)
    parser.add_argument ('--ip-protocol', type = int, help  = 'ip protocol, check IpProtocolType enum', required = False, default = 0)
    parser.add_argument ('--ip-address', type = str, help  = 'ip address', required = False, default = '')
    parser.add_argument ('--serial-port', type = str, help  = 'serial port', required = False, default = '')
    parser.add_argument ('--mac-address', type = str, help  = 'mac address', required = False, default = '')
    parser.add_argument ('--other-info', type = str, help  = 'other info', required = False, default = '')
    parser.add_argument ('--streamer-params', type = str, help  = 'other info', required = False, default = '')
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
    params.timeout = args.timeout

    if (args.log):
        BoardShim.enable_dev_board_logger ()
    else:
        BoardShim.disable_board_logger ()

    board = BoardShim (args.board_id, params)
    board.prepare_session ()

    # board.start_stream () # use this for default options
    board.start_stream (45000, args.streamer_params)
    time.sleep (10)
    # data = board.get_current_board_data (256) # get latest 256 packages or less, doesnt remove them from internal buffer
    data = board.get_board_data () # get all data and remove it from internal buffer
    board.stop_stream ()
    board.release_session ()

    print (data)


if __name__ == "__main__":
    main ()
