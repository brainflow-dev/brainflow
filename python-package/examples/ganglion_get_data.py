import argparse
import time
import brainflow


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--port', type = str, help  = 'port name', required = True)
    parser.add_argument ('--log', action = 'store_true')
    args = parser.parse_args ()

    brainflow.board_shim.BoardShim.enable_dev_board_logger ()

    board = brainflow.board_shim.BoardShim (brainflow.board_shim.GANGLION.board_id, args.port)
    board.prepare_session ()
    board.start_stream ()
    time.sleep (25)
    board.stop_stream ()
    board.release_session ()


if __name__ == "__main__":
    main ()
