# Python
Following commands will install all required packages for you:
```
cd python-package
pip install -e .
``` 
After installation you will be able to use this library via:
```
from brainflow import *
```
Simple example:
```

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
    time.sleep (5)
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
All [BoardShim methods](https://github.com/Andrey1994/brainflow/blob/master/python-package/brainflow/board_shim.py)

All [DataHandler methods](https://github.com/Andrey1994/brainflow/blob/master/python-package/brainflow/preprocess.py)

All possible error codes are described [here](https://github.com/Andrey1994/brainflow/blob/master/python-package/brainflow/exit_codes.py)

For more information and samples please go to [examples](https://github.com/Andrey1994/brainflow/tree/master/python-package/examples)
