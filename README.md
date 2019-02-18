# Brainflow

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG data.

Core module of this library is implemented in C\C++ and available for all bindings as a dynamic library.
## Shared library methods:
```
int prepare_session (int board_id, const char *port_name);
int start_stream (int buffer_size);
int stop_stream ();
int release_session ();
int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
int get_board_data_count (int *result);
int get_board_data (int data_count, float *data_buf, double *ts_buf);
```
For now only [OpenBCI Cython](http://docs.openbci.com/Hardware/02-Cyton) is supported, but architecture allows to add new boards and not only from OpenBCI without any issues

![Cython](https://farm5.staticflickr.com/4817/32567183898_10a4b56659.jpg)

## Bindings
I support bindigs for:
* [python](https://github.com/Andrey1994/brainflow/blob/master/python-package/examples/brainflow_get_data.py)
* [java](https://github.com/Andrey1994/brainflow/blob/master/java-package/brainflow/src/test/java/BrainFlowTest.java)
* [R](https://github.com/Andrey1994/brainflow/blob/master/r-package/examples/brainflow_get_data.R)
* [CPP](https://github.com/Andrey1994/brainflow/blob/master/cpp-package/src/brainflow_get_data.cpp)
* [Matlab](https://github.com/Andrey1994/brainflow/blob/master/matlab-package/brainflow/brainflow_get_data.m)
* [C#](https://github.com/Andrey1994/brainflow/blob/master/csharp-package/brainflow/test/get_board_data.cs)

These bindings just call methods from dynamic libraries 

### Python sample
```
import argparse
from brainflow import *

def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--port', type = str, help  = 'port name', required = True)
    args = parser.parse_args ()

    board = BoardShim (CYTHON.board_id, args.port)
    board.prepare_session ()
    board.start_stream ()
    time.sleep (5)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    data_handler = DataHandler (CYTHON.board_id, numpy_data = data)
    filtered_data = data_handler.preprocess_data (order = 3, start = 1, stop = 50)
    data_handler.save_csv ('results.csv')
    print (filtered_data.head ())
    read_data = DataHandler (CYTHON.board_id, csv_file = 'results.csv')
    print (read_data.get_data ().head ())


if __name__ == "__main__":
    main ()
```

## Applications
### GUI
Brainflow GUI is based on R Shiny package and provides simple UI to monitor EEG\EMG\ECG data
![image1](https://farm2.staticflickr.com/1842/30854740608_e40c6c5248_o_d.png)
### P300 Speller
P300 speller is based on [Event Related Potentials](https://en.wikipedia.org/wiki/Event-related_potential). I use TKInter to draw UI and [LDA](https://scikit-learn.org/stable/modules/generated/sklearn.discriminant_analysis.LinearDiscriminantAnalysis.html) to perform classification
#### Installation
* Electrode placement: P3,P4,C3,C4,T5,T6,O1,O2
* Install [Git LFS](https://git-lfs.github.com/) because I use it to save eeg data and pickled classifier
* Use Python 2.7 x64, and install packages from requirements.txt, to use Python 3 you will need to retrain the classifier(Pickle is not portable between different OSes and Python's versions)
[![Watch the video](https://farm8.staticflickr.com/7811/45713649104_1b32faa349_h.jpg)](https://youtu.be/1GdjMx5t4ls)

## BrainFlow board emulator

This tool was designed to allow developers to contribute to this library even if they don't have an access to the OpenBCI board,it works as a device driver

### Linux
For now only file operations are supported, so you will need to recompile this library
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
