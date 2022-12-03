=====
Usage
=====

SimpleBLE works on Windows, Linux, MacOS and iOS. Please follow the instructions below
to build and run SimpleBLE in your specific environment.


Building and Installing the Library (Source)
============================================

When building SimpleBLE from source, you will need some dependencies based on your
current operating system.

**Linux** ::

  sudo apt install libdbus-1-dev

**Windows** ::

   `Windows SDK <https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/>`_ (Version 10.0.19041.0 or higher)

The included CMake build script can be used to build SimpleBLE.
CMake is freely available for download from https://www.cmake.org/download/. ::

   cd <path-to-simpleble>
   mkdir build && cd build
   cmake .. -DSIMPLEBLE_LOG_LEVEL=[VERBOSE|DEBUG|INFO|WARNING|ERROR|FATAL]
   cmake --build . -j7
   sudo cmake --install .

To build a shared library set the ``BUILD_SHARED_LIBS`` CMake variable to ``TRUE`` ::

  cmake -DBUILD_SHARED_LIBS=TRUE ...


Usage with CMake (Installed)
============================

Once SimpleBLE has been installed, it can be consumed from within CMake::

   find_package(simpleble REQUIRED CONFIG)
   target_link_libraries(<your-target> simpleble::simpleble)


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

  - ``LIBFMT_GIT_TAG``: The git tag to use for fmtlib. *(Default: v8.1.1)*

  - ``LIBFMT_LOCAL_PATH``: The local path to use for fmtlib. *(Default: None)*


Build Examples
==============

Use the following instructions to build the provided SimpleBLE examples: ::

   cd <path-to-repository>
   mkdir build && cd build
   cmake -DSIMPLEBLE_LOCAL=ON ../examples/simpleble
   cmake --build . -j7


Plain-flavored Build
====================

Use the following instructions to build SimpleBLE with the plain-flavored API: ::

   cd <path-to-simpleble>
   mkdir build && cd build
   cmake .. -DSIMPLEBLE_PLAIN=ON
   cmake --build . -j7
   sudo cmake --install .


Testing
=======

To build and run unit and integration tests, the following packages are
required: ::

   sudo apt install libgtest-dev libgmock-dev python3-dev
   pip3 install -r <path-to-simpleble>/test/requirements.txt


Unit Tests
----------

To run the unit tests, run the following command: ::

   cd <path-to-simpleble>
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMPLEBLE_TEST=ON
   cmake --build . -j7
   ./bin/simpleble_test


Address Sanitizer Tests
-----------------------

To run the address sanitizer tests, run the following command: ::

   cd <path-to-simpleble>
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMPLEBLE_SANITIZE=Address -DSIMPLEBLE_TEST=ON
   cmake --build . -j7
   PYTHONMALLOC=malloc ./bin/simpleble_test

It's important for ``PYTHONMALLOC`` to be set to ``malloc``, otherwise the tests will
fail due to Python's memory allocator from triggering false positives.


Thread Sanitizer Tests
----------------------

To run the thread sanitizer tests, run the following command: ::

   cd <path-to-simpleble>
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMPLEBLE_SANITIZE=Thread -DSIMPLEBLE_TEST=ON
   cmake --build . -j7
    ./bin/simpleble_test


.. Links

.. _cmake-init-fetchcontent: https://github.com/friendlyanon/cmake-init-fetchcontent

.. _fmtlib: https://github.com/fmtlib/fmt
