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
board = BoardShim (Boards.Cython.value, args.port)
board.prepare_session ()
board.start_stream ()
time.sleep (5)
data = board.get_board_data ()
board.stop_stream ()
board.release_session ()

data_handler = DataHandler (Boards.Cython.value, data)
filtered_data = data_handler.preprocess_data (1, 50)
filtered_data.to_csv ('results.csv')
```
All [BoardShim methods](https://github.com/Andrey1994/brainflow/blob/master/python-package/brainflow/board_shim.py)

All [DataHandler methods](https://github.com/Andrey1994/brainflow/blob/master/python-package/brainflow/preprocess.py)

All possible error codes are described [here](https://github.com/Andrey1994/brainflow/blob/master/python-package/brainflow/exit_codes.py)

For more information and samples please go to [examples](https://github.com/Andrey1994/brainflow/tree/master/python-package/examples)
