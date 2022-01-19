# SimpleBluez
A simple C++ wrapper around Bluez with a commercially-friendly licence.

## Overview
SimpleBluez is a C++ implementation of the Bluez API on top of [SimpleDBus](https://github.com/OpenBluetoothToolbox/SimpleDBus). It's designed to provide complete access to the underlying Bluez stack, requiring the user to properly handle any associated behaviors and side-effects.

If you want to use the library and need help. **Please reach out!**
You can find me at: `kevin at dewald dot me`

## Build
SimpleBluez should work on any Linux environment. Make sure you have necesary dependencies from SimpleDBus installed.

### Standalone build from source

```
cd <source-root>
mkdir build && cd build
cmake ..
make -j
sudo make install # Not available yet.
```

### Build as part of another project
If you want to add SimpleBluez to your project without any external dependencies, just clone the repository and link to it on your `CMakeLists.txt` file.

```
add_subdirectory(<path-to-simplebluez> ${CMAKE_BINARY_DIR}/simplebluez)
include_directories(${SIMPLEBLUEZ_INCLUDES})
```

### Build examples
```
cd <source-root>
mkdir build && cd build
cmake ../examples
make -j
```

### Build tests
To build and run unit and integration tests, the following packages are required:
`sudo apt install libgtest-dev libgmock-dev python3-dev`

#### Address Sanitizer
In order to run tests with Address Sanitizer, CMake needs to be called with
the following option: `-DSIMPLEBLUEZ_SANITIZE=Address`. It is also important to
set the environment variable `PYTHONMALLOC=malloc` to prevent Python's memory
allocator from triggering false positives.

#### Thread Sanitizer
In order to run tests with Thread Sanitizer, CMake needs to be called with
the following option: `-DSIMPLEBLUEZ_SANITIZE=Thread`.

## Security

One key security feature of the library is it allows the user to specify
the URLs and tags of all internal dependencies, thus allowing compilation
from internal or secure sources without the risk of those getting compromised.
This is done by specifying the additional command line arguments to
the `cmake` command:

- `SIMPLEDBUS_GIT_REPOSITORY`
  Call CMake with `-DSIMPLEDBUS_GIT_REPOSITORY=<path>` to override the
  default location of the SimpleDBus repository.
- `SIMPLEDBUS_GIT_TAG`
  Call CMake with `-DSIMPLEDBUS_GIT_TAG=<tag>` to override the default
  tag of the SimpleDBus repository.
- `LIBFMT_GIT_REPOSITORY`
  Call CMake with `-DLIBFMT_GIT_REPOSITORY=<path>` to override the
  default location of the LibFmt repository.
- `LIBFMT_GIT_TAG`
  Call CMake with `-DLIBFMT_GIT_TAG=<tag>` to override the default
  tag of the LibFmt repository.

Alternatively, instead of specifying the repository and tag, the user can
specify the path to a local version of the dependencies. This is done by
specifying the additional command line arguments to the `cmake` command:

- `SIMPLEDBUS_LOCAL_PATH`
  Call CMake with `-DSIMPLEDBUS_LOCAL_PATH=<path>` to override the
  default location of the SimpleDBus repository.
- `LIBFMT_LOCAL_PATH`
  Call CMake with `-DLIBFMT_LOCAL_PATH=<path>` to override the
  default location of the LibFmt repository.

## More information
- You can find the official documentation for Bluez in [this link](https://git.kernel.org/pub/scm/bluetooth/bluez.git/plain/doc/gatt-api.txt).

## Contributing
If you want to help, here is what's needed:
- More documentation.
- More examples.
- Logging
