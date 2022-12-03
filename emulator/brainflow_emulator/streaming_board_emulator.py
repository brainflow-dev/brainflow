import logging
import subprocess
import sys

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow_emulator.emulate_common import TestFailureError, log_multilines


def run_test(cmd_list):
    logging.info('Running %s' % ' '.join([str(x) for x in cmd_list]))
    process = subprocess.Popen(cmd_list, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()

    log_multilines(logging.info, stdout)
    log_multilines(logging.info, stderr)

    if process.returncode != 0:
        raise TestFailureError('Test failed with exit code %s' % str(process.returncode), process.returncode)

    return stdout, stderr


def main():
    BoardShim.enable_dev_board_logger()
    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.SYNTHETIC_BOARD.value, params)
    board.prepare_session()
    board.add_streamer('streaming_board://225.1.1.1:6677', BrainFlowPresets.DEFAULT_PRESET)
    board.add_streamer('streaming_board://225.1.1.1:6678', BrainFlowPresets.DEFAULT_PRESET)
    board.start_stream()

    run_test(sys.argv[1:])

    board.stop_stream()
    board.release_session()


if __name__ == "__main__":
    logging.basicConfig(format='%(asctime)s %(levelname)-8s %(message)s', level=logging.INFO)
    main()
