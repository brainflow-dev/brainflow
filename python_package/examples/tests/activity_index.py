import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter


def main():
    BoardShim.enable_dev_board_logger()

    # use synthetic board for demo
    params = BrainFlowInputParams()
    board_id = BoardIds.SYNTHETIC_BOARD.value
    board = BoardShim(board_id, params)
    board.prepare_session()
    board.start_stream()
    BoardShim.log_message(LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    sampling_rate = BoardShim.get_sampling_rate(board_id)
    accel_channels = BoardShim.get_accel_channels(board_id)
    print(f'Accel channels: {accel_channels}, Sampling rate: {sampling_rate}')

    # demo activity index calculation per 1-second epoch
    ai = DataFilter.get_activity_index(
        data[accel_channels[0]], data[accel_channels[1]], data[accel_channels[2]],
        sampling_rate)
    print(f'Activity Index per 1-second epoch: {ai}')


if __name__ == '__main__':
    main()
