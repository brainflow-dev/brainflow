---
layout: post
title: BrainFlow 2.0
subtitle: New Major version of BrainFlow
tags: [release, history]
---

API was changes and its not backward compatible!

Major changes in this release:

* move BoardInfoGetter to C++
* move DataHandler to C++
* generate docs for all languages
* add analog mode for Cyton based boards
* pass structure with parameters to identify boards instead generic string
* drop matlab package
* also now we publish packages to nuget

Example:
```python
import argparse
import time
import brainflow
import numpy as np

import pandas as pd
import matplotlib
matplotlib.use ('Agg')
import matplotlib.pyplot as plt

from brainflow.board_shim import BoardShim, BrainFlowInputParams
from brainflow.data_filter import DataFilter, FilterTypes


def main ():
    parser = argparse.ArgumentParser ()
    # use docs to check which parameters are required for specific board, e.g. for Cyton - set serial port,
    parser.add_argument ('--ip-port', type = int, help  = 'ip port', required = False, default = 0)
    parser.add_argument ('--ip-protocol', type = int, help  = 'ip protocol, check IpProtocolType enum', required = False, default = 0)
    parser.add_argument ('--ip-address', type = str, help  = 'ip address', required = False, default = '')
    parser.add_argument ('--serial-port', type = str, help  = 'serial port', required = False, default = '')
    parser.add_argument ('--mac-address', type = str, help  = 'mac address', required = False, default = '')
    parser.add_argument ('--other-info', type = str, help  = 'other info', required = False, default = '')
    parser.add_argument ('--board-id', type = int, help  = 'board id, check docs to get a list of supported boards', required = True)
    parser.add_argument ('--log', action = 'store_true')
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

    # demo how to read data as 2d numpy array
    board = BoardShim (args.board_id, params)
    board.prepare_session ()
    board.start_stream ()
    time.sleep (10)
    # data = board.get_current_board_data (256) # get latest 256 packages or less, doesnt remove them from internal buffer
    data = board.get_board_data () # get all data and remove it from internal buffer
    board.stop_stream ()
    board.release_session ()

    # demo how to convert it to pandas DF and plot data
    eeg_channels = BoardShim.get_eeg_channels (args.board_id)
    df = pd.DataFrame (np.transpose (data))
    print (df.head ())
    plt.figure ()
    df[eeg_channels].plot (subplots = True)
    plt.savefig ('before_processing.png')

    # demo how to perform signal processing
    for count, channel in enumerate (eeg_channels):
        if count == 0:
            DataFilter.perform_bandpass (data[channel], BoardShim.get_sampling_rate (args.board_id), 15.0, 6.0, 4, FilterTypes.BESSEL.value, 0)
        elif count == 1:
            DataFilter.perform_bandstop (data[channel], BoardShim.get_sampling_rate (args.board_id), 5.0, 1.0, 3, FilterTypes.BUTTERWORTH.value, 0)
        elif count == 2:
            DataFilter.perform_lowpass (data[channel], BoardShim.get_sampling_rate (args.board_id), 9.0, 5, FilterTypes.CHEBYSHEV_TYPE_1.value, 1)
        elif count == 3:
            DataFilter.perform_highpass (data[channel], BoardShim.get_sampling_rate (args.board_id), 3.0, 4, FilterTypes.BUTTERWORTH.value, 0)

    df = pd.DataFrame (np.transpose (data))
    print (df.head ())
    plt.figure ()
    df[eeg_channels].plot (subplots = True)
    plt.savefig ('after_processing.png')


if __name__ == "__main__":
    main ()

```

**Update(August, 2021): Despite the fact that current BrainFlow version is 4.5.0, this example more likely still works since there were not many major changes in core API**