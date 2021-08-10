---
layout: post
title: BrainFlow 1.0
subtitle: First Working version of BrainFlow
tags: [release, history]
comments: true
---

First working version, only OpenBCI Cyton and Windows are supported.

In terms of user code it looks like:

```python
import argparse
import time
import brainflow


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--port', type = str, help  = 'port name', required = True)
    parser.add_argument ('--log', action = 'store_true')
    args = parser.parse_args ()

    if (args.log):
        brainflow.board_shim.BoardShim.enable_board_logger ()
    else:
        brainflow.board_shim.BoardShim.disable_board_logger ()

    board = brainflow.board_shim.BoardShim (brainflow.board_shim.CYTHON.board_id, args.port)
    board.prepare_session ()
    board.start_stream ()
    time.sleep (25)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    data_handler = brainflow.preprocess.DataHandler (brainflow.board_shim.CYTHON.board_id, numpy_data = data)
    filtered_data = data_handler.preprocess_data (order = 3, start = 1, stop = 50)
    data_handler.save_csv ('results.csv')
    print (filtered_data.head ())
    read_data = brainflow.preprocess.DataHandler (brainflow.board_shim.CYTHON.board_id, csv_file = 'results.csv')
    print (read_data.get_data ().head ())


if __name__ == "__main__":
    main ()

```

**Update(August, 2021): As you can see the main difference comparing to it's current state is an absence of BrainFlowInputParams and in DataHandler API. Core methods of BoardShim API are still the same as before.**
