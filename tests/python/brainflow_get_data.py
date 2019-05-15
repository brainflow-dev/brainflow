import sys
import time
import brainflow


def main ():
    brainflow.board_shim.BoardShim.enable_board_logger ()

    board = brainflow.board_shim.BoardShim (brainflow.board_shim.CYTON.board_id, sys.argv[1])
    board.prepare_session ()
    board.start_stream ()
    time.sleep (25)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    data_handler = brainflow.preprocess.DataHandler (brainflow.board_shim.CYTON.board_id, numpy_data = data)
    filtered_data = data_handler.preprocess_data (order = 3, start = 1, stop = 50)
    if filtered_data.empty:
        raise Exception ('no data from board')
    data_handler.save_csv ('results.csv')
    print (filtered_data.head ())
    read_data = brainflow.preprocess.DataHandler (brainflow.board_shim.CYTON.board_id, csv_file = 'results.csv')
    print (read_data.get_data ().head ())
    if read_data.get_data ().empty:
        raise Exception ('no data from file')


if __name__ == "__main__":
    main ()
