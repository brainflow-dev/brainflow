import argparse
import time
import brainflow


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--first-port', type = str, help  = 'port name for the first board', required = True)
    parser.add_argument ('--first-board', type = int, help  = 'board id for the first board', required = True)
    parser.add_argument ('--second-port', type = str, help  = 'port name for the second board', required = True)
    parser.add_argument ('--second-board', type = int, help  = 'board id for the second board', required = True)
    parser.add_argument ('--log', action = 'store_true')
    args = parser.parse_args ()

    first_board = brainflow.board_shim.BoardShim (args.first_board, args.first_port)
    second_board = brainflow.board_shim.BoardShim (args.second_board, args.second_port)
    first_board.prepare_session ()
    second_board.prepare_session ()

    if (args.log):
        brainflow.board_shim.BoardShim.enable_dev_board_logger ()
    else:
        brainflow.board_shim.BoardShim.disable_board_logger ()

    first_board.start_stream ()
    second_board.start_stream ()
    time.sleep (10)
    first_data = first_board.get_board_data ()
    second_data = second_board.get_board_data ()
    first_board.stop_stream ()
    second_board.stop_stream ()
    first_board.release_session ()
    #second_board.release_session () test that we handle it correctly

    first_data_handler = brainflow.preprocess.DataHandler (args.first_board, numpy_data = first_data)
    first_filtered_data = first_data_handler.preprocess_data (order = 3, start = 1, stop = 50)
    second_data_handler = brainflow.preprocess.DataHandler (args.second_board, numpy_data = second_data)
    second_filtered_data = second_data_handler.preprocess_data (order = 3, start = 1, stop = 50)


if __name__ == "__main__":
    main ()