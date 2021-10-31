# SimpleDBus
A simple C++ wrapper around DBus with a commercially-friendly licence.

## Overview
SimpleDBus is a C++ low-level binding around DBus designed to be easy to integrate and use with commercial projects. 

This library is designed to be an extremely minimalistic high-level wrapper around DBus,
with most of the focus placed on proper packing/unpacking of DBus data structures. It's
primary intended use case is by [SimpleBluez](https://github.com/OpenBluetoothToolbox/SimpleBluez).

If you want to use the library and need help. **Please reach out!**
You can find me at: `kevin at dewald dot me`

## Build
SimpleDBus should work on any Linux environment using DBus. To install the necessary dependencies on Debian-based systems, use the following command: `sudo apt install libdbus-1-dev`

A set of helper scripts are provided to simplify the process of building the
library, but are not required.

### Standalone build from source

```
cd <path-to-simpledbus>
mkdir build && cd build
cmake .. -DSIMPLEDBUS_LOG_LEVEL=[VERBOSE_3|VERBOSE_2|VERBOSE_1|VERBOSE_0|DEBUG|INFO|WARNING|ERROR|FATAL]
make -j
sudo make install # Not available yet.
```

### Build as part of another project
If you want to add SimpleDBus to your project without any external dependencies, just clone the repository and link to it on your `CMakeLists.txt` file.

```
add_subdirectory(<path-to-simpledbus> ${CMAKE_BINARY_DIR}/simpledbus)
include_directories(${SIMPLEDBUS_INCLUDES})
```

### Build examples
```
cd <path-to-simpledbus>
mkdir build && cd build
cmake ../examples
make -j
```

## License
All components within this project that have not been bundled from external creators, are licensed under the terms of the [MIT Licence](LICENCE.md).
