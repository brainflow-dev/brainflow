from brainflow import *

def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--port', type = str, help  = 'port name', required = True)
    args = parser.parse_args ()

    board = BoardShim (Boards.Cython.value, args.port.encode ())
    board.prepare_session ()
    board.start_stream ()
    time.sleep (5)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    data_handler = DataHandler (Boards.Cython, data)
    filtered_data = data_handler.preprocess_data (1, 50)
    filtered_data.to_csv ('results.csv')


if __name__ == "__main__":
    main ()
