import argparse
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BrainFlowPresets, BoardIds



def main():
    BoardShim.enable_board_logger()

    params1 = BrainFlowInputParams() 
    params1.preset = BrainFlowPresets.DEFAULT_PRESET
    params1.master_board = BoardIds.SYNTHETIC_BOARD
    params1.ip_address = "225.1.1.1"
    params1.ip_port = 6677
    board1 = BoardShim(BoardIds.STREAMING_BOARD, params1)
    board1.prepare_session()
    board1.start_stream()
    time.sleep(5)
    data1 = board1.get_board_data()
    board1.stop_stream()
    board1.release_session()

    print(data1)

    params2 = BrainFlowInputParams() 
    params2.preset = BrainFlowPresets.AUXILIARY_PRESET
    params2.master_board = BoardIds.SYNTHETIC_BOARD
    params2.ip_address = "225.1.1.1"
    params2.ip_port = 6678
    board2 = BoardShim(BoardIds.STREAMING_BOARD, params2)
    board2.prepare_session()
    board2.start_stream()
    time.sleep(5)
    data2 = board2.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
    board2.stop_stream()
    board2.release_session()

    print(data2)


if __name__ == "__main__":
    main()
