import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
from brainflow.data_filter import DataFilter
from brainflow.ml_model import MLModel


def main():
    print('BoardShim version: ' + BoardShim.get_version())
    print('DataFilter version: ' + DataFilter.get_version())
    print('MLModel version: ' + MLModel.get_version())
    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.SYNTHETIC_BOARD.value, params)

    for i in range(2):
        board.prepare_session()
        board.start_stream()
        time.sleep(10)
        board.stop_stream()
        data = board.get_board_data()
        print(DataFilter.calc_stddev(data[2]))
        data = board.get_board_data()
        board.release_session()


if __name__ == "__main__":
    main()
