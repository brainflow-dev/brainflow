# BrainFlow board emulator

This tools were designed to allow developers contribute to this library even if the don't have an access to the OpenBCI board,it works as kernel's module and at least for now supports only file operationas, tty operations may be added in future

## Linux
Execute the following commands with root privileges to run it on Linux machine:
```
cd linux
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
