# SimpleBLE

The ultimate fully-fledged cross-platform BLE library, designed for simplicity
and ease of use.

All specific operating system quirks are handled to provide a consistent behavior
across all platforms. Each major version of the library will have a stable API
that will be fully forwards compatible.

Some basic examples of key features have also been provided.

If you want to use the library and need help. **Please reach out!**
You can find me at: kevin at dewald dot me

## Build Instructions
The project is built using [CMake](http://cmake.org/). The basic main flow is as follows:

```bash
  mkdir build
  cd build
  cmake .. # Replace path to '../examples' to build the examples
  make
```

The build will generate both a static and a shared library.
**TODO:** Expand this part of the documentation.

A set of helper scripts are provided to simplify the process of building the
library, but are not required.

### Windows Requirements
- [Visual Studio 2019](https://visualstudio.microsoft.com/)
  - Make sure you install the C++ toolchain with Windows SDK Build 19041 at least.

**NOTE**: It might be easier to use the bundled helper scripts to build for Windows.

### Linux Requirements
The Linux implementation of SimpleBLE is built upon [SimpleDBus](https://github.com/kdewald/SimpleDBus),
which in turn requires [libdbus](https://dbus.freedesktop.org/doc/dbus-c/latest/).
- `libdbus-1-dev`
  - On Ubuntu: `sudo apt-get install libdbus-1-dev`

**NOTE**: See the [Security](#security) section for more information on how to
override the default paths for external dependencies.

### macOS Requirements
- [Xcode Command Line Tools](https://developer.apple.com/xcode/)

### In-source builds
Building the library from source is possible by cloning the repository and linking
to it on your `CMakeLists.txt` file.

```cmake
add_subdirectory(<path-to-simpleble> ${CMAKE_BINARY_DIR}/simpleble)
include_directories(${SIMPLEBLE_INCLUDES})
```

## Architecture

### Layers and their responsibilities
- External layer
    - `SimpleBLE::Adapter` and `SimpleBLE::Peripheral` classes.
    - These objects hold a shared pointer to `SimpleBLE::AdapterBase` 
      and `SimpleBLE::PeripheralBase` respectively.
- C-style wrapper layer
    - This layer is a C-style wrapper around the safe C++, designed
      to allow integration of SimpleBLE into other languages that
      have support for C bindings.
- Safe layer
    - `SimpleBLE::AdapterSafe` and `SimpleBLE::PeripheralSafe` classes.
    - These objects wrap all `SimpleBLE::Adapter` and `SimpleBLE::Peripheral`
      objects and provide an interface that does not throw exceptions.
      Instead, it will return an `std::optional<T>` object if the function
      returns a value, or a boolean indicating whether the function succeeded
      if the original function did not return a value. The usage is functionally
      equivalent to their respective counterparts in the external layer.
- API layer (OS-dependent)
    - `SimpleBLE::AdapterBase` and `SimpleBLE::PeripheralBase` classes.
    - These classes specify the API of the library on top of which
      the external layer is actually wrapping.
    - Each OS target has to implement the full public API specified in
      the external layer, using private methods and properties for 
      the specific requirements of each environment.
    - Two convenience classes, `SimpleBLE::AdapterBuilder` and
      `SimpleBLE::PeripheralBuilder` are provided for the case of
      allowing access to private methods during the build process.

### Security

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

## Collaborating

### Coding & Naming Conventions
- This project follows the coding style guidelines specified in the `.clang-format` file.
  They are roughly derived from Google's coding standard with a few differences, outlined
  here:
    - Class function names are expressed in `snake_case`.
    - Class protected and private property names must end with an underscore (`_`).
    - Class protected and private method names must start with an underscore (`_`).

### API per OS

The following tables describe the state of each available API per operating system.
Any field that is not specified as supported will throw a compilation error if used
or just be ignored.

| API                                                  | Linux | Windows | MacOS |
| ---------------------------------------------------- | ----- | ------- | ----- |
| `SimpleBLE::Adapter::identifier`                     | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::address`                        | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::scan_start`                     | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::scan_stop`                      | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::scan_for`                       | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::scan_is_active`                 | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::scan_get_results`               | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::set_callback_on_scan_start`     | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::set_callback_on_scan_stop`      | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::set_callback_on_scan_updated`   | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::set_callback_on_scan_found`     | Yes   | Yes     | Yes   |
| `SimpleBLE::Adapter::get_adapters`                   | Yes   | Yes     | Yes   |
| ---------------------------------------------------- | ----- | ------- | ----- |
| `SimpleBLE::Peripheral::identifier`                  | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::address`                     | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::connect`                     | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::disconnect`                  | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::is_connected`                | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::is_connectable`              | No    | No      | Yes   |
| `SimpleBLE::Peripheral::services`                    | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::manufacturer_data`           | No    | No      | No    |
| `SimpleBLE::Peripheral::read`                        | No    | Yes     | Yes   |
| `SimpleBLE::Peripheral::write_request`               | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::write_command`               | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::notify`                      | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::indicate`                    | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::unsubscribe`                 | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::set_callback_on_connected`   | Yes   | Yes     | Yes   |
| `SimpleBLE::Peripheral::set_callback_on_disconnected`| Yes   | Yes     | Yes   |

## Known Issues / To-Do's
- [Linux] Fork safety is not guaranteed.
- [Linux] `SimpleBLE::Peripheral::read` does not work.
- [MacOS] Only the main system adapter can be detected.
- [MacOS] Implementation has incomplete error handling and might crash if invalid parameters are passed.
- [MacOS] Timeout logic can be DRYed up.
- [MacOS] Raise exceptions upon error.
- [MacOS] Service and characteristic UUIDs need to be normalized.
- [Windows] Unclear if multiple adapters can be detected.
- [All] Add a signal handler to ensure all objects are disconnected when the program exits.
- [All] Add safe version of the library that won't trigger any exceptions.
- [All] Replace C-style casts with C++ style casts.
- [All] Add Python bindings for the library.
- [All] Add Javascript bindings for the library.
- [All] Add Kotlin bindings for the library.

## Ideas
- Explore if callbacks can be wrapped in shared pointers to prevent them from being prematurely deleted.
- Explore if callbacks can be handled in a separate thread to prevent blocking the main thread.

## License
All components within this project that have not been bundled from external creators, are licensed under the terms of the [MIT Licence](LICENCE.md).
