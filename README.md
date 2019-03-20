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
int set_log_level (int log_level);
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

## Brainflow Emulator

Brainflow emulator allows users to run tests without EEG headset.

* On Windows it works using [com0com](http://com0com.sourceforge.net/)
* On Linux it works using pty

You should pass command line to test directly to [cython_linux.py](./brainflow_emulator/cython_linux.py) or to [cython_windows.py](./brainflow_emulator/cython_windows.py), script will add port automatically to provided command line and will start an application

### Example

Install emulator package
```
cd emulator
pip3 install -U .
```
Run test
```
python3 brainflow_emulator/cython_linux.py python3 /media/sf_folder/brainflow/python-package/examples/brainflow_get_data.py --log --port
INFO:root:Running python3 /media/sf_folder/brainflow/python-package/examples/brainflow_get_data.py --log --port /dev/pts/1
INFO:root:read "b'v'"
INFO:root:read "b'b'"
INFO:root:read "b's'"
INFO:root:stdout is: b'   package_num         eeg1          eeg2  ...    accel2    accel3     timestamp\n0          0.0    59.692543    264.193723  ...  3.526750  1.616875  1.553124e+09\n1          1.0   425.466525   2773.043094  ...  1.519125 -3.823000  1.553124e+09\n2          2.0  1349.970005  12973.129874  ...  2.586875  0.657750  1.553124e+09\n3          3.0  2730.721543  35263.509116  ... -2.776625  1.778750  1.553124e+09\n4          4.0  4333.067974  60181.588357  ...  3.000500 -0.270625  1.553124e+09\n\n[5 rows x 13 columns]\n   package_num         eeg1          eeg2  ...    accel2    accel3     timestamp\n0          0.0    59.692543    264.193723  ...  3.526750  1.616875  1.553124e+09\n1          1.0   425.466525   2773.043094  ...  1.519125 -3.823000  1.553124e+09\n2          2.0  1349.970005  12973.129874  ...  2.586875  0.657750  1.553124e+09\n3          3.0  2730.721543  35263.509116  ... -2.776625  1.778750  1.553124e+09\n4          4.0  4333.067974  60181.588357  ...  3.000500 -0.270625  1.553124e+09\n\n[5 rows x 13 columns]\n'
INFO:root:stderr is: b'[2019-03-20 19:12:10.313] [board_logger] [info] openning port /dev/pts/1: 0\n'
```

Emulator raises *TestFailureError* if test fails
