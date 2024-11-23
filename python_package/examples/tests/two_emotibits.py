import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()

    parser = argparse.ArgumentParser()
    parser.add_argument('--id1', type=str, help='id for first emotibit', required=True)
    parser.add_argument('--id2', type=str, help='id for second emotibit', required=True)
    args = parser.parse_args()

    params1 = BrainFlowInputParams()
    params1.serial_number = args.id1
    params2 = BrainFlowInputParams()
    params2.serial_number = args.id2
    board_id = BoardIds.EMOTIBIT_BOARD.value

    presets = BoardShim.get_board_presets(board_id)
    print (presets)

    # Init both boards
    board1 = BoardShim(board_id, params1)
    board2 = BoardShim(board_id, params2)
    board1.prepare_session()
    board2.prepare_session()

    # Start streaming for both
    board1.start_stream()
    board2.start_stream()
    time.sleep(10)

    # Get data from both
    data_default1 = board1.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
    data_aux1 = board1.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
    data_anc1 = board1.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)
    data_default2 = board2.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
    data_aux2 = board2.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
    data_anc2 = board1.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)

    # Stop streaming for both
    board1.stop_stream()
    board2.stop_stream()

    # Release both boards
    board1.release_session()
    board2.release_session()

    # Write data from both
    DataFilter.write_file(data_default1, 'default1.csv', 'w')
    DataFilter.write_file(data_aux1, 'aux1.csv', 'w')
    DataFilter.write_file(data_anc1, 'anc1.csv', 'w')
    DataFilter.write_file(data_default2, 'default2.csv', 'w')
    DataFilter.write_file(data_aux2, 'aux2.csv', 'w')
    DataFilter.write_file(data_anc2, 'anc2.csv', 'w')


if __name__ == "__main__":
    main()
