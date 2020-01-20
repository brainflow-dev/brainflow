import argparse
import time
import statistics

import numpy as np
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, LogLevels
from brainflow.data_filter import DataFilter, FilterTypes, AggOperations


def main ():
    parser = argparse.ArgumentParser ()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port
    parser.add_argument ('--ip-port', type = int, help  = 'ip port', required = False, default = 0)
    parser.add_argument ('--ip-protocol', type = int, help  = 'ip protocol, check IpProtocolType enum', required = False, default = 0)
    parser.add_argument ('--ip-address', type = str, help  = 'ip address', required = False, default = '')
    parser.add_argument ('--serial-port', type = str, help  = 'serial port', required = False, default = '')
    parser.add_argument ('--mac-address', type = str, help  = 'mac address', required = False, default = '')
    parser.add_argument ('--other-info', type = str, help  = 'other info', required = False, default = '')
    parser.add_argument ('--streamer-params', type = str, help  = 'other info', required = False, default = '')
    parser.add_argument ('--board-id', type = int, help  = 'board id, check docs to get a list of supported boards', required = True)
    parser.add_argument ('--log', action = 'store_true')
    parser.add_argument ('--run-time', type = int, help = 'run time in sec', required = True)
    args = parser.parse_args ()

    params = BrainFlowInputParams ()
    params.ip_port = args.ip_port
    params.serial_port = args.serial_port
    params.mac_address = args.mac_address
    params.other_info = args.other_info
    params.ip_address = args.ip_address
    params.ip_protocol = args.ip_protocol

    if (args.log):
        BoardShim.enable_dev_board_logger ()
    else:
        BoardShim.disable_board_logger ()

    # for streaming board need to use master board id
    master_board_id = args.board_id
    if args.board_id == BoardIds.STREAMING_BOARD.value:
        master_board_id = int (params.other_info)

    board = BoardShim (args.board_id, params)
    board.prepare_session ()

    buffer_size = int (BoardShim.get_sampling_rate (master_board_id) * args.run_time * 1.2) # + 20% for safety

    board.start_stream (buffer_size, args.streamer_params)
    time.sleep (args.run_time)
    board.stop_stream ()
    data = board.get_board_data ()
    board.release_session ()

    if master_board_id in (BoardIds.CYTON_BOARD.value, BoardIds.CYTON_WIFI_BOARD.value, BoardIds.GANGLION_WIFI_BOARD.value):
        bytes_per_package = 33
    elif master_board_id in (BoardIds.CYTON_DAISY_BOARD, BoardIds.CYTON_DAISY_WIFI_BOARD.value):
        bytes_per_package = 66
    elif master_board_id == BoardIds.SYNTHETIC_BOARD.value:
        bytes_per_package = 104
    elif master_board_id == BoardIds.NOVAXR_BOARD.value:
        bytes_per_package = 72
    else:
        raise ValueError ('unsupported board')

    total_bytes_received = bytes_per_package * data.shape[1]
    packages_per_sec = float (data.shape[1]) / float (args.run_time);

    timestamp_channel = BoardShim.get_timestamp_channel (master_board_id)
    timestamp_array = data[timestamp_channel]
    time_diff_array = list ()
    for i in range (0, timestamp_array.size - 1):
        time_diff_array.append (timestamp_array[i + 1] - timestamp_array[i])

    package_num_channel = BoardShim.get_package_num_channel (master_board_id)
    package_num_array = data[package_num_channel]
    lost_packages = 0
    expected = 0
    cur_id = 0
    while cur_id < package_num_array.size:
        if expected == 256:
            expected = 0
        if package_num_array[cur_id] != expected:
            BoardShim.log_message (LogLevels.LEVEL_WARN.value,
                'package loss detected: position %d package_num value %d expected value %d' % (cur_id, package_num_array[cur_id], expected))
            lost_packages = lost_packages + 1
        else:
            cur_id = cur_id + 1
        expected = expected + 1

    package_loss = (lost_packages / data.shape[1]) * 100

    BoardShim.log_message (LogLevels.LEVEL_INFO.value, '\nResults:\n')
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'package loss percent %f' % package_loss)
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'average time delta %f' % statistics.mean (time_diff_array))
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'std deviation of time delta %f' % statistics.pstdev (time_diff_array))
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'total packages received %d' % data.shape[1])
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'packages per sec %f' % packages_per_sec)
    BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'total bytes received %d' % total_bytes_received)

    eeg_channels = BoardShim.get_eeg_channels (master_board_id)
    emg_channels = BoardShim.get_emg_channels (master_board_id)
    total_channels = eeg_channels
    # for cyton/ganglion eeg_channels and emg_channels are the same array because we can not split it
    # for novaxr its 2 different arrays, join them
    for ch in emg_channels:
        if ch not in total_channels:
            total_channels.append (ch)

    df = pd.DataFrame (np.transpose (data))
    df[total_channels].to_csv ('eeg_emg_data.csv')
    df.to_csv ('all_data.csv')
    plt.figure ()
    df[total_channels].plot (subplots = True)
    plt.show ()


if __name__ == "__main__":
    main ()
