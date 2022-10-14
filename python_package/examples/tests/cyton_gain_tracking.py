import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter, WindowOperations, DetrendOperations


def main():
    BoardShim.enable_dev_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--serial-port', type=str, help='serial port', required=True)
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.serial_port = args.serial_port

    data_streaming_seconds = 5
    board_id = BoardIds.CYTON_DAISY_BOARD
    board_descr = BoardShim.get_board_descr(board_id)
    sampling_rate = int(board_descr['sampling_rate'])
    board = BoardShim(board_id, params)
    nfft = DataFilter.get_nearest_power_of_two(sampling_rate)
    eeg_channels = board_descr['eeg_channels']

    channel_chars = ["1", "2", "3", "4", "5", "6", "7", "8", "Q", "W", "E", "R", "T", "Y", "U", "I"]
    gain_chars = ["6", "5", "4", "3", "2", "1", "0"]
    input_type_char = "5"
    gain_test_all_channels = []

    try:
        for channel_index, channel in enumerate(channel_chars):

            if channel_index > 1:
                break
            gain_test_per_channel = []

            for gain_index, gain in enumerate(gain_chars):
                board.prepare_session()
                config_string = f"x{channel}0{gain}{input_type_char}110X"
                resp = board.config_board(config_string)
                print(resp)
                # check that there is a response if streaming is off
                if not resp:
                    raise ValueError('resp is None')
                board.start_stream()
                time.sleep(data_streaming_seconds)
                data = board.get_board_data()
                board.stop_stream()
                eeg_channel = eeg_channels[channel_index]
                DataFilter.detrend(data[eeg_channel], DetrendOperations.LINEAR.value)
                psd = DataFilter.get_psd_welch(data[eeg_channel], nfft, nfft // 2, sampling_rate,
                                            WindowOperations.BLACKMAN_HARRIS.value)
                band_power_injected_current = DataFilter.get_band_power(psd, 30.0, 33.0)
                gain_test_per_channel.append(band_power_injected_current)
                board.release_session()

            gain_test_all_channels.append(gain_test_per_channel)
            
    finally:
        for channel in gain_test_all_channels:
            print(*channel)
        if board.is_prepared():
            board.release_session()
            

if __name__ == "__main__":
    main()