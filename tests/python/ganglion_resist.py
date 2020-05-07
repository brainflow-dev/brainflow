import argparse
import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--serial-port', type = str, help  = 'serial port', required = False, default = '')
    parser.add_argument ('--mac-address', type = str, help  = 'mac address', required = False, default = '')
    args = parser.parse_args ()

    BoardShim.enable_dev_board_logger ()

    params = BrainFlowInputParams ()
    params.serial_port = args.serial_port;
    params.mac_address = args.mac_address;
    board = BoardShim (BoardIds.GANGLION_BOARD.value, params)
    board.prepare_session ()

    # expected result: 5 seconds of resistance data after that 5 seconds of exg data
    board.config_board ('z')
    board.start_stream (45000, 'file://raw_data.csv:w')
    time.sleep (5)
    board.config_board ('Z')
    time.sleep (5)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    print (data)


if __name__ == "__main__":
    main ()
