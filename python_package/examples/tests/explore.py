import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BrainFlowPresets, BoardIds
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()
    parser = argparse.ArgumentParser()
    parser.add_argument('--mac-address', type=str, help='mac address', required=False, default='')
    args = parser.parse_args()

    params = BrainFlowInputParams()
    params.mac_address = args.mac_address
    board = BoardShim(BoardIds.EXPLORE_4_CHAN_BOARD, params)
    board.prepare_session()
    board.start_stream()
    # important: for explore device config board has to be after start stream
    # board.config_board('sampling_rate:500')
    # board.config_board('test_signal:1') # 1 is a bitmask represented as int not channel num
    time.sleep(20)
    data_eeg = board.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
    data_orn = board.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
    data_env = board.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)
    board.stop_stream()
    board.release_session()
    
    print(data_eeg)
    print(data_orn)
    print(data_env)
    DataFilter.write_file(data_eeg, 'data_eeg.csv', 'w')
    DataFilter.write_file(data_orn, 'data_orn.csv', 'w')
    DataFilter.write_file(data_env, 'data_env.csv', 'w')


if __name__ == "__main__":
    main()
