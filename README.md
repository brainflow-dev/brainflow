# Brainflow

Brainflow is a library intended to obtain and analyze EEG and EMG data from OpenBCI boards

It is written on C\C++ and provides a dynamic library which used by different bindings

DLL interface:
```
int prepare_session (int board_id, const char *port_name);
int start_stream (int buffer_size);
int stop_stream ();
int release_session ();
int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
int get_board_data_count (int *result);
int get_board_data (int data_count, float *data_buf, double *ts_buf);
```
For now only linux and Cython serial board are supported, but Windows support will be added soon as well as other boards
We are going to add Java/Matlab/R/NodeJS bindings and more examples in the next versions

## Python
It's not required to compile the entire library to use Python binding.
Following commands will install all required packages for you:
```
cd python
pip3.6 install -e .
``` 
It depends only on numpy but some Python examples require to install additional libraries
After installation you will be able to use it via:
```
from brainflow import *
```
And obtain data using:
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
For more detailed description af Python binding and examples please visit [Python](https://github.com/Andrey1994/brainflow/tree/master/python)

## Brainflow Emulator
These tools were designed to allow developers contribute to this library even if the don't have an access to the OpenBCI board,it works as kernel's module and at least for now supports only file operationas, tty operations may be added in future

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
./cmake_build_emu.sh
cd python
pip install -e .
```
