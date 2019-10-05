import argparse
import time
import brainflow

import matplotlib
matplotlib.use ('Agg')
import matplotlib.pyplot as plt


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--port', type = str, help  = 'port name, for synthetic board port_name doesnt matter, just pass smth', required = True)
    parser.add_argument ('--board', type = int, help  = 'board id, check docs to get a list of supported boards', required = True)
    parser.add_argument ('--log', action = 'store_true')
    args = parser.parse_args ()

    if (args.log):
        brainflow.board_shim.BoardShim.enable_dev_board_logger ()
    else:
        brainflow.board_shim.BoardShim.disable_board_logger ()

    board = brainflow.board_shim.BoardShim (args.board, args.port)
    board.prepare_session ()
    board.start_stream ()
    time.sleep (25)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    data_handler = brainflow.preprocess.DataHandler (args.board, numpy_data = data)
    data_handler.remove_dc_offset ()
    data_handler.notch_interference ()
    filtered_data = data_handler.get_data ()
    #filtered_data = data_handler.preprocess_data (order = 3, start = 1, stop = 50)
    # plot eeg channels
    eeg_columns = list ()
    for col_name in filtered_data.columns:
        if col_name.startswith ('eeg'):
            eeg_columns.append (col_name)
    plt.figure ()
    filtered_data[eeg_columns].plot (subplots = True)
    plt.savefig('plot.png')
    data_handler.save_csv ('results.csv')
    print (filtered_data.head ())


if __name__ == "__main__":
    main ()
