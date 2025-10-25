import time
from pprint import pprint

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BrainFlowPresets, BoardIds
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.GALEA_BOARD, params)
    board.prepare_session()
    board.start_stream()
    time.sleep(10)
    data_default = board.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
    data_aux = board.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
    board.stop_stream()
    board.release_session()

    DataFilter.write_file(data_eeg, 'data_default.csv', 'w')
    DataFilter.write_file(data_orn, 'data_aux.csv', 'w')

    # To get info about channels and presets
    for preset in BoardShim.get_board_presets(BoardIds.GALEA_BOARD):
        preset_description = BoardShim.get_board_descr(BoardIds.GALEA_BOARD, preset)
        pprint(preset_description)


if __name__ == "__main__":
    main()
