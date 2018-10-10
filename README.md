# Brainflow

Brainflow is a library intended to obtain and analyze EEG data.

Core module of this library is implemented in C\C++ and available for all bindings as a dynamic library.
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

## GUI
Brainflow GUI is a R Shiny application which uses Brainflow R package to obtain data from BCI board
### Screenshot:
![image1](https://farm2.staticflickr.com/1842/30854740608_e40c6c5248_o_d.png)

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
## Bindings
### Python
Following commands will install all required packages for you:
```
cd python-package
pip install -e .
``` 
After installation you will be able to use this library via:
```
from brainflow import *
```
#### Example
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

Some [examples](https://github.com/Andrey1994/brainflow/tree/master/python-package/examples)

### R
This package works via [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html) which allows to call Python function directly from R, also it translates all Python types(even user defined) to the correspond R types.

#### Example:
```
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

Also [GUI](https://github.com/Andrey1994/brainflow/tree/master/gui) is implemented using brainflow R package and Shiny module

### Cpp
Headers and compiled libraries are located in ./cpp-package/inc and ./cpp-package/lib directories respectively.
You are able to use ./cpp-package/src as a reference

To compile this project on Linux:
```
mkdir build
cd build
cmake ..
make
```
You are able to use msbuild as well and compile it for Windows

### Matlab
For some Matlab's versions and OSes you may need to recompile brainflow using specific compiler.
For example loadlibrary in Matlab 2017 works only if library was compiled with gcc < 5.0

#### Example
```
board_shim = BoardShim (BoardsIds.CYTHON_BOARD, '/dev/ttyUSB0')
ec = board_shim.prepare_session ()
board_shim.check_ec (ec)
ec = board_shim.start_stream (3600)
board_shim.check_ec (ec)
pause (5)
ec = board_shim.stop_stream ()
board_shim.check_ec (ec)
[ec, data, ts] = board_shim.get_current_board_data (250)
board_shim.check_ec (ec)
disp (data)
ec = board_shim.release_session ()
board_shim.check_ec (ec)
```
[BoardShim matlab class](https://github.com/Andrey1994/brainflow/blob/master/matlab-package/brainflow/BoardShim.m)

### Java
Java binding for brainflow is a Maven project, which calls C methods using JNA

#### Example:
```
BoardShim board_shim = new BoardShim (Boards.CYTHON, "/dev/ttyUSB0");
board_shim.prepare_session ();
board_shim.start_stream (3600);
Thread.sleep (1000);
board_shim.stop_stream ();
System.out.println (board_shim.get_board_data_count ());
System.out.println (board_shim.get_board_data ());
board_shim.release_session ();
```
Note: For some OS you may need to set jna.library.path properly

