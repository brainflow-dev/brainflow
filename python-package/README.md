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
# create board object and allocate resources
board = CythonBoard (b'/dev/ttyUSB0)
board.prepare_session ()
# start streaming
board.start_stream ()
#collect some data
time.sleep (15)
# get last 250 datapoints (this method doesn't free buffer, last 250 samples will still exist)
current_data = board.get_current_board_data (250)
# get all data (this method free the buffer and returns all samples as numpy array)
data = board.get_board_data ()
# stop streaming and free resources
board.stop_stream ()
board.release_session ()
```

Board methods:
```
class CythonBoard (object):

    def __init__ (self, port_name):
    def prepare_session (self):
    def start_stream (self, num_samples = 3600*250):
    def stop_stream (self):
    def release_session (self):
    def get_current_board_data (self, num_samples = 250 * 2):
    def get_immediate_board_data (self):
    def get_board_data_count (self):
    def get_board_data (self):
```
All possible error codes are described [here](https://github.com/Andrey1994/brainflow/blob/master/python-package/brainflow/exit_codes.py)

For more information and samples please go to [examples](https://github.com/Andrey1994/brainflow/tree/master/python-package/examples)
