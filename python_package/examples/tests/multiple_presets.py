import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BrainFlowPresets, BoardIds



def main():
    BoardShim.enable_board_logger()

    params = BrainFlowInputParams() 
    board = BoardShim(BoardIds.SYNTHETIC_BOARD, params)
    board.prepare_session()
    # to test with streaming board
    board.start_stream(45000, "streaming_board://225.1.1.1:6677")
    board.add_streamer("streaming_board://225.1.1.1:6678", BrainFlowPresets.AUXILIARY_PRESET)
    # to test with playback
    # board.start_stream(45000, "file://data_default.csv:w")
    # board.add_streamer("file://data_aux.csv:w", BrainFlowPresets.AUXILIARY_PRESET)
    time.sleep(30)
    data = board.get_board_data()
    board.stop_stream()
    board.release_session()


if __name__ == "__main__":
    main()
