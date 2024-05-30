import socket
import threading
import time
import logging
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowError
from brainflow.data_filter import DataFilter, FilterTypes, NoiseTypes

def stream_data(conn, board):
    try:
        while True:
            data = board.get_current_board_data(250)
            # Apply filters and remove artifacts
            for channel_data in data:
                DataFilter.perform_bandpass(channel_data, 250, 0.5, 50.0, 4, FilterTypes.BUTTERWORTH.value, 0)
                DataFilter.remove_environmental_noise(channel_data, 250, NoiseTypes.FIFTY.value)
            conn.sendall(data.tobytes())
            time.sleep(1)
    except BrokenPipeError:
        logging.info("Client disconnected, waiting for new connection...")
    except Exception as e:
        logging.error(f"Error while streaming data: {e}")
    finally:
        conn.close()

def handle_client(conn, board):
    stream_data(conn, board)

def main():
    logging.basicConfig(level=logging.INFO)
    BoardShim.enable_dev_board_logger()
    log_level = 0  # TRACE level logging
    BoardShim.set_log_level(log_level)

    params = BrainFlowInputParams()
    params.serial_port = '/dev/spidev0.0'

    try:
        logging.info("Preparing session with serial port: {}".format(params.serial_port))
        board = BoardShim(BoardIds.PIEEG_BOARD.value, params)
        board.prepare_session()
        logging.info("Session prepared successfully.")
        board.start_stream(45000, '')

        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind(('0.0.0.0', 64677))
        server.listen(5)
        logging.info("Server listening on port 64677")

        stop_event = threading.Event()

        def accept_clients():
            while not stop_event.is_set():
                try:
                    server.settimeout(1)
                    conn, addr = server.accept()
                    logging.info(f"Connected by {addr}")
                    client_thread = threading.Thread(target=handle_client, args=(conn, board))
                    client_thread.start()
                except socket.timeout:
                    continue

        accept_thread = threading.Thread(target=accept_clients)
        accept_thread.start()

        input("Press Enter to stop the server...\n")
        stop_event.set()
        accept_thread.join()

    except BrainFlowError as e:
        logging.error("Error: {} unable to prepare session".format(str(e)))
    except Exception as e:
        logging.error(f"Unexpected error: {e}")
    finally:
        try:
            logging.info("Stopping stream...")
            board.stop_stream()
            logging.info("Stream stopped.")
        except BrainFlowError as stop_error:
            logging.error("Error stopping the stream: {}".format(str(stop_error)))
        try:
            logging.info("Releasing session...")
            board.release_session()
            logging.info("Session released.")
        except BrainFlowError as release_error:
            logging.error("Error releasing the session: {}".format(str(release_error)))
        except Exception as e:
            logging.error(f"Unexpected error during cleanup: {e}")

if __name__ == "__main__":
    main()
