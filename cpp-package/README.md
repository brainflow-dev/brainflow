# CPP example
Headers and compiled libraries are located in ./inc and ./lib directories respectively
You are able to use ./src as a reference

In fact [board_shim.cpp](https://github.com/Andrey1994/brainflow/blob/master/cpp-package/src/board_shim.cpp) duplicates methods from DLL but wraps them in Class

To compile this project:
```
mkdir build
cd build
cmake ..
make
```
You are able to use msbuild as well and compile it for Windows
