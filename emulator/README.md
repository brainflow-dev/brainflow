# Brainflow emulator allows users to run tests without EEG headset.

## Brainflow Emulator (Cyton)

* On Windows it works using [com0com](http://com0com.sourceforge.net/)
* On Linux it works using pty

You should pass command line to test directly to [cyton_linux.py](./brainflow_emulator/cyton_linux.py) or to [cyton_windows.py](./brainflow_emulator/cyton_windows.py), script will add port automatically to provided command line and will start an application

### Example

Install emulator package
```
cd emulator
pip3 install -U .
```
Run test
```
python3 brainflow_emulator/cyton_linux.py python3 /media/sf_folder/brainflow/python-package/examples/brainflow_get_data.py --log --board 0 --port
INFO:root:Running python3 /media/sf_folder/brainflow/python-package/examples/brainflow_get_data.py --log --board 0 --port /dev/pts/1
INFO:root:read "b'v'"
INFO:root:read "b'b'"
INFO:root:read "b's'"
INFO:root:stdout is: b'   package_num         eeg1          eeg2  ...    accel2    accel3     timestamp\n0          0.0    59.692543    264.193723  ...  3.526750  1.616875  1.553124e+09\n1          1.0   425.466525   2773.043094  ...  1.519125 -3.823000  1.553124e+09\n2          2.0  1349.970005  12973.129874  ...  2.586875  0.657750  1.553124e+09\n3          3.0  2730.721543  35263.509116  ... -2.776625  1.778750  1.553124e+09\n4          4.0  4333.067974  60181.588357  ...  3.000500 -0.270625  1.553124e+09\n\n[5 rows x 13 columns]\n   package_num         eeg1          eeg2  ...    accel2    accel3     timestamp\n0          0.0    59.692543    264.193723  ...  3.526750  1.616875  1.553124e+09\n1          1.0   425.466525   2773.043094  ...  1.519125 -3.823000  1.553124e+09\n2          2.0  1349.970005  12973.129874  ...  2.586875  0.657750  1.553124e+09\n3          3.0  2730.721543  35263.509116  ... -2.776625  1.778750  1.553124e+09\n4          4.0  4333.067974  60181.588357  ...  3.000500 -0.270625  1.553124e+09\n\n[5 rows x 13 columns]\n'
INFO:root:stderr is: b'[2019-03-20 19:12:10.313] [board_logger] [info] openning port /dev/pts/1: 0\n'
```

Emulator raises *TestFailureError* if test fails

## Brianflow Emulator (Ganglion)

To emulate Ganglion device you need to compile mock from GanglionBLEAPI folder and put compiled libraries instead original files like GanglionLibNative64.dll, GanglionLibNative32.dll and the same for Linux\MacOs libraries, rebuild brainflow just to put new libraries to correct locations(these libraries will be loaded at runtime so if you know target location you are able just copypaste them with recompilation of brainflow project) and run as with real board.

## Brainflow Emulator (NovaXR)

NovaXR emulator creates socket server and streams data to brainflow like it's a real board but with much lower sampling rate

### Example
Install emulator package
```
cd emulator
pip3 install -U .
```
Run test
```
python3 brainflow_emulator/novaxr.py python3 /media/sf_folder/brainflow/python-package/examples/brainflow_get_data.py --log --port 127.0.0.1 --board 3
```