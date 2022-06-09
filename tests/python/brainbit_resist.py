import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds


def main():
    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.BRAINBIT_BOARD.value, params)
    board.prepare_session()

    board.start_stream(45000, 'file://raw_data.csv:w')
    # its a little tricky and unclear, need to call start_stream to create data acquisition thread(it also sends CommandStartSignal)
    # after that send CommandStartResist, CommandStopResist and to get EEG data call CommandStartSignal manually
    board.config_board('CommandStartResist')
    time.sleep(5)
    board.config_board('CommandStopResist')
    board.config_board('CommandStartSignal')
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    print(data)


if __name__ == "__main__":
    main()
