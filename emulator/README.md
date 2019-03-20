# Brainflow Emulator

Brainflow emulators allows to run tests without EEG headset.

* On Windows it works using com0com
* On Linux it works using pty

You should pass command line to test directly to cython_linux.py or to cython_writer.py, script will add port automatically to provided command line and will start an application

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

