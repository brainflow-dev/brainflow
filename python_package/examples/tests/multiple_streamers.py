import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()

    # use synthetic board for demo
    params = BrainFlowInputParams()
    board_id = BoardIds.SYNTHETIC_BOARD.value

    presets = BoardShim.get_board_presets(board_id)
    print (presets)
    
    board = BoardShim(board_id, params)
    board.prepare_session()
    board.add_streamer('file://streamer_default.csv:w')
    board.add_streamer('streaming_board://225.1.1.1:6677')
    board.start_stream()
    board.add_streamer('streaming_board://225.1.1.1:6678', BrainFlowPresets.AUXILIARY_PRESET)
    board.add_streamer('file://streamer_aux.csv:w', BrainFlowPresets.AUXILIARY_PRESET)
    time.sleep(10)
    board.delete_streamer('streaming_board://225.1.1.1:6678', BrainFlowPresets.AUXILIARY_PRESET)
    board.delete_streamer('streaming_board://225.1.1.1:6677')
    time.sleep(5)
    data_default = board.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
    data_aux = board.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
    board.stop_stream()
    board.release_session()
    DataFilter.write_file(data_default, 'default.csv', 'w')
    DataFilter.write_file(data_aux, 'aux.csv', 'w')


if __name__ == "__main__":
    main()
