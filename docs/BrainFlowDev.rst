BrainFlow Dev
===============

Code style
-----------

We use clang-format tool to keep the same code style for all cpp files. You can download clang-format binary from `LLVM Download Page <http://releases.llvm.org/download.html>`_
We recommend installing a plugin for your text editor or IDE which will apply clang-format tool during saving. You will need to set code style option to "FILE"

Plugins for text editors and IDEs:

- `Sublime <https://packagecontrol.io/packages/Clang%20Format>`_
- `VSCode <https://marketplace.visualstudio.com/items?itemName=xaver.clang-format>`_
-  clang-format tool is preinstalled for Visual Studio


Unfortunately clang-format cannot handle naming, so some additional rules are:
- methods and variables should be in lower case with underscore
- class names should be in camel case
- use brackets even for single line if and for statements

We try to keep the same code style for all bindings as well, even if it doesn't match PEP or other standards. For example we add spaces before and after assignment operator to specify default value for method's params and add spaces before brackets.

CI and tests
--------------

If you want to commit to the core module of BrainFlow project please check that all tests are passed, you should check CI status in your PR and fix all errors if any. Also, you are able to run failed tests locally using BrainFlow emulator.

Pull Requests
--------------

Just try to briefly explain a goal of this PR.

Instructions to add new boards to BrainFlow
---------------------------------------------

- add new board Id to `BoardIds enum in C code <https://github.com/brainflow-dev/brainflow/blob/master/src/utils/inc/brainflow_constants.h>`_ and to the same enum in all bindings
- add new object creation to `board controller C interface <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/board_controller.cpp>`_
- inherit your board from `Board class <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/inc/board.h>`_ and implement all pure virtual methods, store data in DataBuffer object, use `synthetic board <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/inc/synthetic_board.h>`_ as a reference, try to reuse code from `utils <https://github.com/brainflow-dev/brainflow/tree/master/src/utils>`_ folder
- add information about your board to `brainflow_boards.h <https://github.com/brainflow-dev/brainflow/tree/master/src/board_controller/inc/brainflow_boards.h>`_
- add new files to BOARD_CONTROLLER_SRC variable in `CmakeLists.txt <https://github.com/brainflow-dev/brainflow/blob/master/CMakeLists.txt>`_, you may also need to add new directory to *target_include_directories* for BOARD_CONTROLLER_NAME variable

**You've just written Python, Java, C#, R, C++ ... SDKs for your board! Also, now you can use your new board with applications and frameworks built on top of BrainFlow API.**

Optional: We use CI to run tests automatically, to add your board to CI pipelines you can develop a simple emulator for your device. Use `emulators for existing boards <https://github.com/brainflow-dev/brainflow/tree/master/emulator/brainflow_emulator>`_ as a reference and add tests for your device to Github Acttions workflows.

Instructions to build docs locally
------------------------------------

Don't push changes to Docs without local verification.

- install `pandoc <https://pandoc.org/installing.html>`_
- optional: install Doxygen, skip it if you dont understand what it is or don't need to publish your local build

Install requirements::

    cd docs
    python -m pip install -r requirements.txt

Build docs::

    make html

Debug BrainFlow's errors
---------------------------

Since bindings just call methods from dynamic libraries, more likely errors occur in C++ code, it means that you need to use C++ debuger like gdb. If there is an error in binding, it should be simple to figure out and resolve the issue using language specific tools.

Steps to get more information about errors in C++ code:

- build BrainFlow's core module and C++ binding in debug mode. In files like tools/build_linux.sh default config is Release, so you need to change it to Debug
- reproduce your issue using C++ binding
- run it with debuger and memory checker

Example for Linux(for MacOS it's the same)::

    vim tools/build_linux.sh
    # Change build type to Debug
    bash tools/build_linux.sh
    # Create a test to reproduce your issue in C++, here we will use get_data_demo
    cd tests/cpp/get_data_demo
    mkdir build
    cd build
    cmake -DCMAKE_PREFIX_PATH=TYPE_FULL_PATH_TO_BRAINFLOW_INSTALLED_FOLDER -DCMAKE_BUILD_TYPE=Debug ..
    # e.g. cmake -DCMAKE_PREFIX_PATH=/home/andrey/brainflow/installed_linux -DCMAKE_BUILD_TYPE=Debug ..
    make
    # Run Valgrind to check memory errors
    # Here we use command line for Ganglion
    sudo valgrind --error-exitcode=1 --leak-check=full ./brainflow_get_data --board-id 1 --serial-port /dev/ttyACM0 --mac-address e6:73:73:18:09:b1
    # Valgrind will print Error Summary and exact line numbers
    # Run gdb and get backtrace
    sudo gdb --args ./brainflow_get_data --board-id 1 --serial-port /dev/ttyACM0 --mac-address e6:73:73:18:09:b1
    # In gdb terminal type 'r' to run the program and as soon as error occurs, type 'bt' to see backtrace with exact lines of code and call stack


BrainFlow Emulator
--------------------

BrainFlow Emulator allows you to run all integration tests for all supported boards without real hardware. Our CI uses it for test automation. Also, you can run it on your own PC!

Emulators listed here intended for CI and run process to test automatically. It's great for automation but not easy to use if you need to test it in GUI. So, for some devices there are manual emulators as well. Such emulators don't run process to test for you. Manual emulators make it easier to run tests in GUI based applications.

Streaming Board
~~~~~~~~~~~~~~~~~~

Streaming Board emulator works using Python binding for BrainFlow, so **you need to install Python binding first.**

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests ::

    python emulator\brainflow_emulator\streaming_board_emulator.py python tests\python\brainflow_get_data.py --log --board-id -2 --ip-address 225.1.1.1 --ip-port 6677 --other-info -1

This emulator uses synthetic board as a master board and, IP address and port are hardcoded.

OpenBCI Cyton
~~~~~~~~~~~~~~~

Cyton emulator simulate COM port using:

- `com0com <http://com0com.sourceforge.net/>`_ for Windows
- pty for Linux and MacOS

You should pass test command line directly to cyton_linux.py or to cyton_windows.py. The script will add the port automatically to provided command line and will start an application.


Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests for Linux\MacOS and Windows (port argument will be added by Emulator!) ::

    python brainflow_emulator/cyton_linux.py python ../tests/python/brainflow_get_data.py --log --board-id 0 --serial-port
    python brainflow_emulator\cyton_windows.py python ..\tests\python\brainflow_get_data.py --log --board-id 0 --serial-port


Galea
~~~~~~~

Galea emulator creates socket server and streams data to BrainFlow like it's a real board.

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests::

    python brainflow_emulator/galea_udp.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 3

OpenBCI Wifi Shield based boards
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Wifi shield emulator starts http server to read commands and creates client socket to stream data.

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests for Ganglion, Cyton and Daisy with Wifi Shield::

    python brainflow_emulator/wifi_shield_emulator.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 4 --ip-protocol 2 --ip-port 17982
    python brainflow_emulator/wifi_shield_emulator.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 5 --ip-protocol 2 --ip-port 17982
    python brainflow_emulator/wifi_shield_emulator.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 6 --ip-protocol 2 --ip-port 17982

FreeEEG32
~~~~~~~~~~~

FreeEEG32 emulator simulate COM port using:

- `com0com <http://com0com.sourceforge.net/>`_ for Windows
- pty for Linux and MacOS

You should pass test command line directly to freeeeg32_linux.py or to freeeeg32_windows.py. The script will add the port automatically to provided command line and will start an application.


Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests for Linux\MacOS and Windows (port argument will be added by Emulator!) ::

    python brainflow_emulator/freeeeg32_linux.py python ../tests/python/brainflow_get_data.py --log --board-id 17 --serial-port
    python brainflow_emulator\freeeeg32_windows.py python ..\tests\python\brainflow_get_data.py --log --board-id 17 --serial-port
