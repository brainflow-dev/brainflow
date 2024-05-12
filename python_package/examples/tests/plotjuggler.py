import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds


def main():
    BoardShim.enable_dev_board_logger()
    parser = argparse.ArgumentParser()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port
    parser.add_argument('--timeout', type=int, help='timeout for device discovery or connection', required=False,
                        default=0)
    parser.add_argument('--ip-port', type=int, help='ip port', required=False, default=0)
    parser.add_argument('--ip-protocol', type=int, help='ip protocol, check IpProtocolType enum', required=False,
                        default=0)
    parser.add_argument('--ip-address', type=str, help='ip address', required=False, default='')
    parser.add_argument('--serial-port', type=str, help='serial port', required=False, default='')
    parser.add_argument('--mac-address', type=str, help='mac address', required=False, default='')
    parser.add_argument('--other-info', type=str, help='other info', required=False, default='')
    parser.add_argument('--serial-number', type=str, help='serial number', required=False, default='')
    parser.add_argument('--board-id', type=int, help='board id, check docs to get a list of supported boards',
                        required=True)
    parser.add_argument('--file', type=str, help='file', required=False, default='')
    parser.add_argument('--master-board', type=int, help='master board id for streaming and playback boards',
                        required=False, default=BoardIds.NO_BOARD)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.ip_port = args.ip_port
    params.serial_port = args.serial_port
    params.mac_address = args.mac_address
    params.other_info = args.other_info
    params.serial_number = args.serial_number
    params.ip_address = args.ip_address
    params.ip_protocol = args.ip_protocol
    params.timeout = args.timeout
    params.file = args.file
    params.master_board = args.master_board

    board = BoardShim(args.board_id, params)
    board_id = board.get_board_id()
    presets = BoardShim.get_board_presets(board_id)
    board.prepare_session()
    for preset in presets:
        # to stream to plotjuggler
        board.add_streamer(
            'plotjuggler_udp://127.0.0.1:9870', preset)
        # to store data in a file
        board.add_streamer(f'file://data_{preset}.csv:w', preset)
    board.start_stream ()

    while True:
        try:
            time.sleep(1)
        except KeyboardInterrupt:
            break
        except Exception as e:
            print(e)
    board.release_session()


if __name__ == "__main__":
    main()