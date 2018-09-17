# Brainflow

Brainflow is a library intended to obtain and analyze EEG, EMG and ECG data.

I've implemented the core module of this library in C\C++ and compiled it in DLL, so all difficult logic and package parsing
was done only once in native code, while other languages just call methods from DLL
## DLL interface:
```
int prepare_session (int board_id, const char *port_name);
int start_stream (int buffer_size);
int stop_stream ();
int release_session ();
int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
int get_board_data_count (int *result);
int get_board_data (int data_count, float *data_buf, double *ts_buf);
```
For now only Cython serial board is supported, more boards will be added soon.

## GUI
Brainflow GUI is a R Shiny application which use Brainflow R package to obtain data from BCI board
### Screenshot:
![image1](https://farm2.staticflickr.com/1842/30854740608_e40c6c5248_o_d.png)

## Python
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


## BrainFlow board emulator

These tools were designed to allow developers contribute to this library even if the don't have an access to the OpenBCI board,it works as a kernel's module which implements file operations

### Linux
Execute the following commands with root privileges to run it on Linux machine:
```
cd emulator/linux
make
./module_load.sh
```
After that you will be able to read/write data to/from /dev/emulated_cython device
Also you will have to build main library in emulated mode
```
cd %brainflow_dir%
./cmake_build.sh EMULATOR
# you will have to reinstall packages
cd python-package
pip install -e .
```

## R
This package works via [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html) which allows to call Python function directly from R, also it translates all Python classes(even user defined) to the corresponds R constructures.

For now it is not added to the CRAN, so you will have to build this package manually using R-strudio

### Example:
```
library(reticulate)
# you have to change it and make sure you have brainflow-python installed
# after pushing brainflow to PYPI and CRAN it will not be necessary
use_python("/home/osboxes/venv_brainflow/bin/python")
library(brainflow)

board_shim <- get_board_shim(Boards()$Cython["Type"], "/dev/emulated_cython")
prepare_session(board_shim)
start_stream(board_shim, 3600)
Sys.sleep(time = 5)
stop_stream(board_shim)
data <- get_current_board_data(board_shim, 250)

data_handler <- get_data_handler(Boards()$Cython["Type"], data)
preprocess_data(data_handler, 1, 50, TRUE)
preprocessed_data <- get_data(data_handler)
head(preprocessed_data)
release_session(board_shim)
```
All methods provided by this package can be found [here](https://github.com/Andrey1994/brainflow/tree/master/r-package/brainflow/R).

Also [GUI](https://github.com/Andrey1994/brainflow/tree/master/gui) is implemented using brainflow R package and Shiny

## TODO List:
* Add Matlab\Octave binding
* Add Java\Scala binding
* Add Nodejs binding
* Add more boards
* Add reading from file to DataHandler
* Replace file_operations by tty_operations in Linux emulator
* Add Windows emulator
* CI\CD
* Push to CRAN, PYPI...
* Package GUI to standalone desktop aplication
