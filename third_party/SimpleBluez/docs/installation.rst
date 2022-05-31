Installation
------------

SimpleBluez should work on any Linux environment. Make sure you have the
necesary dependencies from SimpleDBus installed.

A set of helper scripts are provided to simplify the process of building
the library, but are not required.

Standalone build from source
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

   cd <path-to-simplebluez>
   mkdir build && cd build
   cmake ..
   make
   sudo make install # Not available yet.

Build as part of another project
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you want to add SimpleDBus to your project without any external
dependencies, just clone the repository and link to it on your
``CMakeLists.txt`` file.

::

   add_subdirectory(<path-to-simplebluez> ${CMAKE_BINARY_DIR}/simplebluez)
   include_directories(${SIMPLEBLUEZ_INCLUDES})

Build examples
~~~~~~~~~~~~~~

::

   cd <path-to-simplebluez>
   mkdir build && cd build
   cmake ../examples
   make

Build tests
~~~~~~~~~~~

To build and run unit and integration tests, the following packages are
required: ``sudo apt install libgtest-dev libgmock-dev python3-dev``

Address Sanitizer
^^^^^^^^^^^^^^^^^

In order to run tests with Address Sanitizer, CMake needs to be called
with the following option: ``-DSIMPLEBLUEZ_SANITIZE=Address``. It is also
important to set the environment variable ``PYTHONMALLOC=malloc`` to
prevent Python's memory allocator from triggering false positives.

Thread Sanitizer
^^^^^^^^^^^^^^^^

In order to run tests with Thread Sanitizer, CMake needs to be called
with the following option: ``-DSIMPLEBLUEZ_SANITIZE=Thread``.
