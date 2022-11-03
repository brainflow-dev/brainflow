=====
Usage
=====

SimpleBluez should work on any Linux environment supporting DBus. To install
the necessary dependencies on Debian-based systems, use the following command: ::

  sudo apt install libdbus-1-dev


Building and Installing the Library (Source)
============================================

The included CMake build script can be used to build SimpleBluez.
CMake is freely available for download from https://www.cmake.org/download/. ::

   cd <path-to-simplebluez>
   mkdir build && cd build
   cmake .. -DSIMPLEBLUEZ_LOG_LEVEL=[VERBOSE|DEBUG|INFO|WARNING|ERROR|FATAL]
   cmake --build . -j7
   sudo cmake --install .

To build a shared library set the ``BUILD_SHARED_LIBS`` CMake variable to ``TRUE`` ::

  cmake -DBUILD_SHARED_LIBS=TRUE ...


Usage with CMake (Installed)
============================

Once SimpleBluez has been installed, it can be consumed from within CMake::

   find_package(simplebluez REQUIRED CONFIG)
   target_link_libraries(<your-target> simplebluez::simplebluez)


Usage with CMake (Local)
=============================

You can add the ``simplebluez`` library directory into your project and include it in
your ``CMakeLists.txt`` file ::

   add_subdirectory(<path-to-simplebluez> ${CMAKE_BINARY_DIR}/simplebluez)
   target_link_libraries(<your-target> simplebluez::simplebluez)


Usage with CMake (Vendorized)
=============================

If you want to use a vendorized copy of SimpleBluez, you can do so by using FetchContent
and specifying the location from where SimpleBluez should be consumed from. ::

   include(FetchContent)
   FetchContent_Declare(
       simplebluez
       GIT_REPOSITORY <simplebluez-git-repository>
       GIT_TAG <simplebluez-git-tag>
       GIT_SHALLOW YES
   )

   # Note that here we manually do what FetchContent_MakeAvailable() would do,
   # except to ensure that the dependency can also get what it needs, we add
   # custom logic between the FetchContent_Populate() and add_subdirectory()
   # calls.
   FetchContent_GetProperties(simplebluez)
   if(NOT simplebluez_POPULATED)
       FetchContent_Populate(simplebluez)
       list(APPEND CMAKE_MODULE_PATH "${simplebluez_SOURCE_DIR}/cmake/find")
       add_subdirectory("${simplebluez_SOURCE_DIR}/simplebluez" "${simplebluez_BINARY_DIR}")
   endif()

   set(simplebluez_FOUND 1)

You can put this code inside ``Findsimplebluez.cmake`` and add it to your CMake
module path, as depicted in `cmake-init-fetchcontent`_.

Once vendorized using the above approach, you can consume SimpleBluez from
within CMake as you'd normally do ::

   find_package(simplebluez REQUIRED)
   target_link_libraries(<your-target> simplebluez::simplebluez)

One key security feature of SimpleBluez is that it allows the user to specify
the URLs and tags of all internal dependencies, thus allowing compilation
from internal or secure sources without the risk of those getting compromised.

Currently, the following libraries are included as part of SimpleBluez, with
the following CMake options available:

- `fmtlib`_

  - ``LIBFMT_VENDORIZE``: Enable vendorization of fmtlib. *(Default: True)*

  - ``LIBFMT_GIT_REPOSITORY``: The git repository to use for fmtlib.

  - ``LIBFMT_GIT_TAG``: The git tag to use for fmtlib. *(Default: v8.1.1)*

  - ``LIBFMT_LOCAL_PATH``: The local path to use for fmtlib. *(Default: None)*


Build Examples
==============

Use the following instructions to build the provided SimpleBluez examples: ::

   cd <path-to-repository>
   mkdir build && cd build
   cmake -DSIMPLEBLUEZ_LOCAL=ON ../examples/simplebluez
   cmake --build . -j7


Testing
=======

To build and run unit and integration tests, the following packages are
required: ::

   sudo apt install libgtest-dev libgmock-dev python3-dev
   pip3 install -r <path-to-simplebluez>/test/requirements.txt


Unit Tests
----------

To run the unit tests, run the following command: ::

   cd <path-to-simplebluez>
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMPLEBLUEZ_TEST=ON
   cmake --build . -j7
   ./bin/simplebluez_test


Address Sanitizer Tests
-----------------------

To run the address sanitizer tests, run the following command: ::

   cd <path-to-simplebluez>
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMPLEBLUEZ_SANITIZE=Address -DSIMPLEBLUEZ_TEST=ON
   cmake --build . -j7
   PYTHONMALLOC=malloc ./bin/simplebluez_test

It's important for ``PYTHONMALLOC`` to be set to ``malloc``, otherwise the tests will
fail due to Python's memory allocator from triggering false positives.


Thread Sanitizer Tests
----------------------

To run the thread sanitizer tests, run the following command: ::

   cd <path-to-simplebluez>
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DSIMPLEBLUEZ_SANITIZE=Thread -DSIMPLEBLUEZ_TEST=ON
   cmake --build . -j7
    ./bin/simplebluez_test


.. Links

.. _cmake-init-fetchcontent: https://github.com/friendlyanon/cmake-init-fetchcontent

.. _fmtlib: https://github.com/fmtlib/fmt
