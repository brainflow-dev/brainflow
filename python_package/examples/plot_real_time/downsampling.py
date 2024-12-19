import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds
from brainflow.data_filter import DataFilter, AggOperations


def main():
    BoardShim.enable_dev_board_logger()

    # use synthetic board for demo
    params = BrainFlowInputParams()
    params.mac_address =   '84:BA:20:6E:3C:1E'#'60:77:71:74:E6:B7'#
    params.timeout  = 40  # board = BoardShim(BoardIds.SYNTHETIC_BOARD.value, params)
    board = BoardShim(59, params)
    board.prepare_session()
    board.start_stream()
    BoardShim.log_message(LogLevels.LEVEL_INFO.value, 'start sleeping in the main thread')
    time.sleep(10)
    data = board.get_board_data(20)
    print(data)
    board.stop_stream()
    board.release_session()

    # eeg_channels = BoardShim.get_eeg_channels(BoardIds.SYNTHETIC_BOARD.value)
    eeg_channels = BoardShim.get_eeg_channels(59)
    # demo for downsampling, it just aggregates data
    for count, channel in enumerate(eeg_channels):
        print('Original data for channel %d:' % channel)
        print(data[channel])
        if count == 0:
            downsampled_data = DataFilter.perform_downsampling(data[channel], 3, AggOperations.MEDIAN.value)
        elif count == 1:
            downsampled_data = DataFilter.perform_downsampling(data[channel], 2, AggOperations.MEAN.value)
        else:
            downsampled_data = DataFilter.perform_downsampling(data[channel], 2, AggOperations.EACH.value)
        print('Downsampled data for channel %d:' % channel)
        print(downsampled_data)


if __name__ == "__main__":
    main()
