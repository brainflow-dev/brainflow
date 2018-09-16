# BrainFlow board emulator

These tools were designed to allow developers contribute to this library even if the don't have an access to the OpenBCI board,it works as a kernel's module which implements file operations

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
./cmake_build.sh EMULATOR
# you will have to reinstall packages
cd python-package
pip install -e .
```
