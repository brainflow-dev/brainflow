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
    parser.add_argument ('--run-time', type = int, help = 'run time for one iteration in sec', required = True)
    parser.add_argument ('--num-iters', type = int, help = 'number of iterations', default = 1)
    parser.add_argument ('--channels', type = str, help = 'channels to plot in format 0,1,2 by default plot all channels', default = None)
    parser.add_argument ('--config-file', type = str, help = 'file with strings to send to device', default = None)
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
    if args.config_file:
        with open (args.config_file) as file:
            lines = file.readlines ()
            for line in lines:
                board.config_board (line)

    buffer_size = int (BoardShim.get_sampling_rate (master_board_id) * args.run_time * 1.2) # + 20% for safety

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

    timestamp_channel = BoardShim.get_timestamp_channel (master_board_id)
    package_num_channel = BoardShim.get_package_num_channel (master_board_id)

    try:
        cur_id = 0
        for i in range (args.num_iters):
            # wait for an input
            input ('Press Enter to continue...')
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, '\nRunning iteration %d/%d\n' % (i, args.num_iters))

            # start stream and get data
            board.start_stream (buffer_size, args.streamer_params)
            time.sleep (args.run_time)
            board.stop_stream ()
            data = board.get_board_data ()

            if data.shape[1] == 0:
                BoardShim.log_message (LogLevels.LEVEL_WARN.value, '\nNo data received!\n')
                continue

            # calculate some metrics
            total_bytes_received = bytes_per_package * data.shape[1]
            packages_per_sec = float (data.shape[1]) / float (args.run_time);
            
            timestamp_array = data[timestamp_channel]
            time_diff_array = list ()
            for j in range (0, timestamp_array.size - 1):
                time_diff_array.append (timestamp_array[j + 1] - timestamp_array[j])


            package_num_array = data[package_num_channel]
            lost_packages = 0
            expected = package_num_array[0]
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

            # provide results for iteration
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, '\nResults:\n')
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'package loss percent %f' % package_loss)
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'average time delta %f' % statistics.mean (time_diff_array))
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'std deviation of time delta %f' % statistics.pstdev (time_diff_array))
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'total packages received %d' % data.shape[1])
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'packages per sec %f' % packages_per_sec)
            BoardShim.log_message (LogLevels.LEVEL_INFO.value, 'total bytes received %d' % total_bytes_received)

            # plot data
            eeg_channels = BoardShim.get_eeg_channels (master_board_id)
            emg_channels = BoardShim.get_emg_channels (master_board_id)
            total_channels = list ()
            if args.channels is not None:
                selected_channels = [int (x) for x in args.channels.split (',')]
                temp_channels = eeg_channels
                for ch in emg_channels:
                    if ch not in temp_channels:
                        temp_channels.append (ch)
                temp_channels = sorted (temp_channels)
                for j in range (len (temp_channels)):
                    if j in selected_channels:
                        total_channels.append (temp_channels[j])
            else:
                # for cyton/ganglion eeg_channels and emg_channels are the same array because we can not split it
                # for novaxr its 2 different arrays, join them
                total_channels = eeg_channels
                for ch in emg_channels:
                    if ch not in total_channels:
                        total_channels.append (ch)
                total_channels = sorted (total_channels)

            try:
                total_channels.extend (BoardShim.get_ppg_channels (master_board_id))
                total_channels.extend (BoardShim.get_eda_channels (master_board_id))
            except:
                pass
            total_channels.append (timestamp_channel)

            columns = list ()
            for j in range (len (total_channels) - 1):
                columns.append ('channel_%d' % (int(total_channels[j]) - 1))
            columns.append ('timestamp')

            df = pd.DataFrame (np.transpose (data))
            df.to_csv ('all_data_%d.csv' % i)

            df_to_plot = df[total_channels]
            df_to_plot.columns = columns
            df_to_plot.to_csv ('selected_data_%d.csv' % i)
            df_to_plot.plot (subplots = True, x = 'timestamp')
            plt.show ()
    finally:
        # release session in the end
        board.release_session ()


if __name__ == "__main__":
    main ()