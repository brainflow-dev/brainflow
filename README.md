# Brainflow

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG data.

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
For now only [OpenBCI Cython](http://docs.openbci.com/Hardware/02-Cyton) is supported
## Applications
### GUI
Brainflow GUI is based on R Shiny package and provides simple UI to monitor EEG\EMG\ECG data
![image1](https://farm2.staticflickr.com/1842/30854740608_e40c6c5248_o_d.png)
### P300 Speller
P300 speller is based on [Event Related Potentials](https://en.wikipedia.org/wiki/Event-related_potential). I use TKInter to draw UI and [LDA](https://scikit-learn.org/stable/modules/generated/sklearn.discriminant_analysis.LinearDiscriminantAnalysis.html) to perform classification
[![Watch the video](https://farm5.staticflickr.com/4890/46128583321_5879580b5c_b.jpg)](https://youtu.be/DPnBzbUbkM4)

## BrainFlow board emulator

This tool was designed to allow developers to contribute to this library even if the don't have an access to the OpenBCI board,it works as a device driver

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
## Bindings
I support bindigs for:
* [python](https://github.com/Andrey1994/brainflow/blob/master/python-package/examples/brainflow_get_data.py)
* [java](https://github.com/Andrey1994/brainflow/blob/master/java-package/brainflow/src/test/java/BrainFlowTest.java)
* [matlab](https://github.com/Andrey1994/brainflow/blob/master/matlab-package/brainflow/brainflow_get_data.m)
* [R](https://github.com/Andrey1994/brainflow/blob/master/r-package/examples/brainflow_get_data.R)
* [CPP](https://github.com/Andrey1994/brainflow/blob/master/cpp-package/src/brainflow_get_data.cpp)
* [Matlab](https://github.com/Andrey1994/brainflow/blob/master/matlab-package/brainflow/brainflow_get_data.m)


It's easy to add new language because all difficult logic about data obtaining is implemented in C\C++ and bindings just call DLL's methods 
