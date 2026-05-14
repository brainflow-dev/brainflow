import argparse
import time

from brainflow.board_shim import (
    BoardIds,
    BoardShim,
    BrainFlowInputParams,
    BrainFlowPresets,
)


def main():
    BoardShim.enable_dev_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--timeout', type=int, help='timeout for device discovery or connection',
                        required=False, default=0)
    parser.add_argument('--mac-address', type=str, help='mac address', required=False, default='')
    parser.add_argument('--serial-number', type=str, help='device name', required=False, default='')
    parser.add_argument('--other-info', type=str, help='other info', required=False, default='')
    parser.add_argument('--plotjuggler-ip', type=str, help='PlotJuggler UDP host',
                        required=False, default='127.0.0.1')
    parser.add_argument('--plotjuggler-port', type=int, help='PlotJuggler UDP port',
                        required=False, default=9870)
    parser.add_argument('--buffer-size', type=int, help='BrainFlow ring buffer size',
                        required=False, default=450000)
    parser.add_argument('--duration', type=float,
                        help='stream duration in seconds; omit or set 0 to stream until Ctrl+C',
                        required=False, default=0.0)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.timeout = args.timeout
    params.mac_address = args.mac_address
    params.serial_number = args.serial_number
    params.other_info = args.other_info

    board_id = BoardIds.MUSE_S_ATHENA_BOARD
    preset = BrainFlowPresets.DEFAULT_PRESET
    streamer = f'plotjuggler_udp://{args.plotjuggler_ip}:{args.plotjuggler_port}'

    board = BoardShim(board_id, params)
    streaming = False
    try:
        eeg_channels = BoardShim.get_eeg_channels(board_id, preset)
        eeg_names = BoardShim.get_eeg_names(board_id)
        sampling_rate = BoardShim.get_sampling_rate(board_id, preset)
        print(f'Muse S Athena EEG channels: {eeg_channels}')
        print(f'Muse S Athena EEG names: {eeg_names}')
        print(f'Muse S Athena EEG sampling rate: {sampling_rate} Hz')
        print(f'Streaming {preset.name} to {streamer}')

        board.prepare_session()
        board.add_streamer(streamer, preset)
        board.start_stream(args.buffer_size)
        streaming = True

        if args.duration > 0:
            time.sleep(args.duration)
        else:
            while True:
                time.sleep(1)
    except KeyboardInterrupt:
        print('Streaming interrupted by user, stopping session.')
    finally:
        if streaming:
            board.stop_stream()
        board.release_session()


if __name__ == '__main__':
    main()
