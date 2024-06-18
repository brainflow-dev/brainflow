import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds


def main():
    BoardShim.enable_dev_board_logger()

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.GANGLION_NATIVE_BOARD.value, params)
    board.prepare_session()
    # expected result: 5 seconds of resistance data(unknown sampling rate) after that 5 seconds of exg data
    board.config_board('z')
    board.start_stream(45000, 'file://raw_data.csv:w')
    time.sleep(5)
    board.config_board('Z')
    time.sleep(5)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()

    print(data)

    resistance_channels = BoardShim.get_resistance_channels(BoardIds.GANGLION_NATIVE_BOARD.value)
    print(resistance_channels)


if __name__ == "__main__":
    main()
