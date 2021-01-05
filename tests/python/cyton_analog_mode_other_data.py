import argparse
import time
import numpy as np

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, FilterTypes


def main():
    parser = argparse.ArgumentParser()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port
    parser.add_argument('--ip-port', type=int, help='ip port', required=False, default=0)
    parser.add_argument('--ip-protocol', type=int, help='ip protocol, check IpProtocolType enum', required=False,
                        default=0)
    parser.add_argument('--ip-address', type=str, help='ip address', required=False, default='')
    parser.add_argument('--serial-port', type=str, help='serial port', required=False, default='')
    parser.add_argument('--mac-address', type=str, help='mac address', required=False, default='')
    parser.add_argument('--other-info', type=str, help='other info', required=False, default='')
    parser.add_argument('--streamer-params', type=str, help='other info', required=False, default='')
    parser.add_argument('--board-id', type=int, help='board id, check docs to get a list of supported boards',
                        required=True)
    parser.add_argument('--log', action='store_true')
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.ip_port = args.ip_port
    params.serial_port = args.serial_port
    params.mac_address = args.mac_address
    params.other_info = args.other_info
    params.ip_address = args.ip_address
    params.ip_protocol = args.ip_protocol

    if (args.log):
        BoardShim.enable_dev_board_logger()
    else:
        BoardShim.disable_board_logger()

    board = BoardShim(args.board_id, params)
    board.prepare_session()

    board.start_stream()
    time.sleep(5)
    board.config_board('/2')  # enable analog mode only for Cyton Based Boards!
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    """
    data[BoardShim.get_other_channels(args.board_id)[0]] contains cyton end byte
    data[BoardShim.get_other_channels(args.board_id)[1....]] contains unprocessed bytes
    if end byte is 0xC0 there are accel data in data[BoardShim.get_accel_channels(args.board_id)[....]] else there are zeros
    if end byte is 0xC1 there are analog data in data[BoardShim.get_analog_channels(args.board_id)[....]] else there are zeros
    """
    print(data[BoardShim.get_other_channels(args.board_id)[0]][0:5])  # should be standard end byte 0xC0
    print(data[BoardShim.get_other_channels(args.board_id)[0]][-5:])  # should be analog and byte 0xC1

    DataFilter.write_file(data, 'cyton_data.csv', 'w')


if __name__ == "__main__":
    main()
