=====
Usage
=====

SimpleBLE works on Windows, Linux, MacOS and iOS. Please follow the instructions below
to build and run SimpleBLE in your specific environment.


System Requirements
===================

When building SimpleBLE from source, you will need some dependencies based on your
current operating system.

**NOTE:** WSL does not support Bluetooth.

General Requirements
--------------------

   - `CMake`_ (Version 3.21 or higher)

Linux
-----

APT-based Distros
~~~~~~~~~~~~~~~~~

   - `libdbus-1-dev` (install via ``sudo apt install libdbus-1-dev``)

RPM-based Distros
~~~~~~~~~~~~~~~~~

   - `dbus-devel`
      - On Fedora, install via ``sudo dnf install dbus-devel``
      - On CentOS, install via ``sudo yum install dbus-devel``

Windows
-------

   - `Windows SDK` (Version 10.0.19041.0 or higher)

MacOS
-----

   - `Xcode Command Line Tools` (install via ``xcode-select --install``)

Android
-------

   - `Android Studio`
   - `Android NDK` (Version 25 or higher. Older versions might work but haven't been thoroughly tested.)


Building and Installing SimpleBLE (Source)
============================================

Compiling the library is done using `CMake`_ and relies heavily on plenty of CMake
functionality. It is strongly suggested that you get familiarized with CMake before
blindly following the instructions below.


Building SimpleBLE
------------------

You can use the following commands to build SimpleBLE: ::

   cmake -S <path-to-simpleble> -B build_simpleble
   cmake --build build_simpleble -j7

Note that if you want to modify the build configuration, you can do so by passing
additional arguments to the ``cmake`` command. For example, to build a shared library
set the ``BUILD_SHARED_LIBS`` CMake variable to ``TRUE`` ::

   cmake -S <path-to-simpleble> -B build_simpleble -DBUILD_SHARED_LIBS=TRUE

To build a plain-flavored version of the library, set the ``SIMPLEBLE_PLAIN`` CMake
variable to ``TRUE`` ::

   cmake -S <path-to-simpleble> -B build_simpleble -DSIMPLEBLE_PLAIN=TRUE

To modify the log level, set the ``SIMPLEBLE_LOG_LEVEL`` CMake variable to one of the
following values: ``VERBOSE``, ``DEBUG``, ``INFO``, ``WARN``, ``ERROR``, ``FATAL`` ::

   cmake -S <path-to-simpleble> -B build_simpleble -DSIMPLEBLE_LOG_LEVEL=DEBUG

**(Linux only)** To force the usage of the DBus session bus, enable the ``SIMPLEBLE_USE_SESSION_DBUS`` flag ::

   cmake -S <path-to-simplebluez> -B build_simplebluez -DSIMPLEBLE_USE_SESSION_DBUS=TRUE

Installing SimpleBLE
--------------------

To install SimpleBLE, you can use the following commands: ::

   cmake --install build_simpleble

Note that if you want to modify the installation configuration, you can do so by passing
additional arguments to the ``cmake`` command. For example, to install the library to
a specific location, set the ``CMAKE_INSTALL_PREFIX`` CMake variable to the desired
location ::

   cmake --install build_simpleble --prefix /usr/local

Note that on Linux and MacOS, you will need to run the ``cmake --install`` command
with ``sudo`` privileges. ::

   sudo cmake --install build_simpleble


Usage with CMake (Installed)
============================

Once SimpleBLE has been installed, it can be consumed from within CMake::

   find_package(simpleble REQUIRED CONFIG)
   target_link_libraries(<your-target> simpleble::simpleble)

Note that this example assumes that SimpleBLE has been installed to a location
that is part of the default CMake module path.


Usage with CMake (Local)
=============================

You can add the ``simpleble`` library directory into your project and include it in
your ``CMakeLists.txt`` file ::

   add_subdirectory(<path-to-simpleble> ${CMAKE_BINARY_DIR}/simpleble)
   target_link_libraries(<your-target> simpleble::simpleble)


Usage with CMake (Vendorized)
=============================

If you want to use a vendorized copy of SimpleBLE, you can do so by using FetchContent
and specifying the location from where SimpleBLE should be consumed from. ::

   include(FetchContent)
   FetchContent_Declare(
       simpleble
       GIT_REPOSITORY <simpleble-git-repository>
       GIT_TAG <simpleble-git-tag>
       GIT_SHALLOW YES
   )

   # Note that here we manually do what FetchContent_MakeAvailable() would do,
   # except to ensure that the dependency can also get what it needs, we add
   # custom logic between the FetchContent_Populate() and add_subdirectory()
   # calls.
   FetchContent_GetProperties(simpleble)
   if(NOT simpleble_POPULATED)
       FetchContent_Populate(simpleble)
       list(APPEND CMAKE_MODULE_PATH "${simpleble_SOURCE_DIR}/cmake/find")
       add_subdirectory("${simpleble_SOURCE_DIR}/simpleble" "${simpleble_BINARY_DIR}")
   endif()

   set(simpleble_FOUND 1)

You can put this code inside ``Findsimpleble.cmake`` and add it to your CMake
module path, as depicted in `cmake-init-fetchcontent`_.

Once vendorized using the above approach, you can consume SimpleBLE from
within CMake as you'd normally do ::

   find_package(simpleble REQUIRED)
   target_link_libraries(<your-target> simpleble::simpleble)

One key security feature of SimpleBLE is that it allows the user to specify
the URLs and tags of all internal dependencies, thus allowing compilation
from internal or secure sources without the risk of those getting compromised.

Currently, the following libraries are included as part of SimpleBLE, with
the following CMake options available:

- `fmtlib`_

  - ``LIBFMT_VENDORIZE``: Enable vendorization of fmtlib. *(Default: True)*

  - ``LIBFMT_GIT_REPOSITORY``: The git repository to use for fmtlib.

  - ``LIBFMT_GIT_TAG``: The git tag to use for fmtlib. *(Default: v9.1.0)*

  - ``LIBFMT_LOCAL_PATH``: The local path to use for fmtlib. *(Default: None)*


Usage alongside native code in Android
======================================

When using SimpleBLE alongside native code in Android, you must include a small
Android dependency module that includes some necessary bridge classes used by SimpleBLE.
This is required because the Android JVM doesn't allow programatic definition of
derived classes, which forces us to bring these definitions in externally.

To include this dependency module, add the following to your `settings.gradle` file:

```groovy
includeBuild("path/to/simpleble/src/backends/android/simpleble-bridge") {
    dependencySubstitution {
        substitute module("org.simpleble.android.bridge:simpleble-bridge") with project(":")
    }
}
```

```kotlin
includeBuild("path/to/simpleble/src/backends/android/simpleble-bridge") {
    dependencySubstitution {
        substitute(module("org.simpleble.android.bridge:simpleble-bridge")).using(project(":"))
    }
}
```

**NOTE:** We will provide Maven packages in the future.


Build Examples
==============

Use the following instructions to build the provided SimpleBLE examples: ::

   cmake -S <path-to-simpleble>/examples/simpleble -B build_simpleble_examples -DSIMPLEBLE_LOCAL=ON
   cmake --build build_simpleble_examples -j7


Testing
=======

To build and run unit and integration tests, the following packages are
required: ::

   sudo apt install libgtest-dev libgmock-dev python3-dev
   pip3 install -r <path-to-simpleble>/test/requirements.txt


Unit Tests
----------

To run the unit tests, run the following command: ::

   cmake -S <path-to-simpleble> -B build_simpleble_test -DSIMPLEBLE_TEST=ON
   cmake --build build_simpleble_test -j7
   ./build_simpleble_test/bin/simpleble_test


Address Sanitizer Tests
-----------------------

To run the address sanitizer tests, run the following command: ::

   cmake -S <path-to-simpleble> -B build_simpleble_test -DSIMPLEBLE_SANITIZE=Address -DSIMPLEBLE_TEST=ON
   cmake --build build_simpleble_test -j7
   PYTHONMALLOC=malloc ./build_simpleble_test/bin/simpleble_test

It's important for ``PYTHONMALLOC`` to be set to ``malloc``, otherwise the tests will
fail due to Python's memory allocator from triggering false positives.


Thread Sanitizer Tests
----------------------

To run the thread sanitizer tests, run the following command: ::

   cmake -S <path-to-simpleble> -B build_simpleble_test -DSIMPLEBLE_SANITIZE=Thread -DSIMPLEBLE_TEST=ON
   cmake --build build_simpleble_test -j7
   ./build_simpleble_test/bin/simpleble_test


.. Links

.. _CMake: https://cmake.org/

.. _Windows SDK: https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk

.. _cmake-init-fetchcontent: https://github.com/friendlyanon/cmake-init-fetchcontent

.. _fmtlib: https://github.com/fmtlib/fmt
