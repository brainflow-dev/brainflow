import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BrainFlowPresets, BoardIds
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_board_logger()

    params = BrainFlowInputParams() 
    board = BoardShim(BoardIds.EXPLORE_4_CHAN_BOARD, params)
    board.prepare_session()
    board.start_stream()
    time.sleep(10)    
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
