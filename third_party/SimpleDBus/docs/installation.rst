Installation
------------


SimpleDBus should work on any Linux environment using DBus. To install
the necessary dependencies on Debian-based systems, use the following
command: ``sudo apt install libdbus-1-dev``

A set of helper scripts are provided to simplify the process of building
the library, but are not required.

Standalone build from source
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

   cd <path-to-simpledbus>
   mkdir build && cd build
   cmake .. -DSIMPLEDBUS_LOG_LEVEL=[VERBOSE_3|VERBOSE_2|VERBOSE_1|VERBOSE_0|DEBUG|INFO|WARNING|ERROR|FATAL]
   make -j
   sudo make install # Not available yet.

Build as part of another project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you want to add SimpleDBus to your project without any external
dependencies, just clone the repository and link to it on your
``CMakeLists.txt`` file.

::

   add_subdirectory(<path-to-simpledbus> ${CMAKE_BINARY_DIR}/simpledbus)
   include_directories(${SIMPLEDBUS_INCLUDES})

Build examples
~~~~~~~~~~~~~~

::

   cd <path-to-simpledbus>
   mkdir build && cd build
   cmake ../examples
   make -j

Build tests
~~~~~~~~~~~

To build and run unit and integration tests, the following packages are
required: ``sudo apt install libgtest-dev libgmock-dev python3-dev``

Coverage
^^^^^^^^

In order to run tests with coverage analysis, CMake needs to be called 
with the following option: ``-DSIMPLEDBUS_COVERAGE=On``. The coverage 
analysis target is called ``simpledbus_test_coverage``

Address Sanitizer
^^^^^^^^^^^^^^^^^

In order to run tests with Address Sanitizer, CMake needs to be called
with the following option: ``-DSIMPLEDBUS_SANITIZE=Address``. It is also
important to set the environment variable ``PYTHONMALLOC=malloc`` to
prevent Python's memory allocator from triggering false positives.

Thread Sanitizer
^^^^^^^^^^^^^^^^

In order to run tests with Thread Sanitizer, CMake needs to be called
with the following option: ``-DSIMPLEDBUS_SANITIZE=Thread``.
