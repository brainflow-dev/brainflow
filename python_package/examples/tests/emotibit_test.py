import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()
    board_id = BoardIds.EMOTIBIT_BOARD.value

    presets = BoardShim.get_board_presets(board_id)
    print (presets)

    board = BoardShim(board_id, params)
    board.prepare_session()
    board.start_stream()
    time.sleep(10)
    data_default = board.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
    data_aux = board.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
    data_anc = board.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)
    board.stop_stream()
    board.release_session()
    DataFilter.write_file(data_default, 'default.csv', 'w')
    DataFilter.write_file(data_aux, 'aux.csv', 'w')
    DataFilter.write_file(data_anc, 'anc.csv', 'w')


if __name__ == "__main__":
    main()
