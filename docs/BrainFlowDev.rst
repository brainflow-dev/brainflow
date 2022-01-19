BrainFlow Dev
===============

We encourage contributions to BrainFlow project from individuals at all levels - students, postdocs, academics, industry coders, etc. Knowledge of the domain of signal processing and/or neurotechx is helpful but not required.

If you want to pick up and try an existing improvement project, you will note that we have indicated levels of difficulty with labels. We recommend you to pick a task marked by `good first issue label <https://github.com/brainflow-dev/brainflow/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22>`_. Most internal hacks will require knowledge of C/C++. Also, we are looking for Rust developers. Knowledge of other languages is useful for binding development.

Navigation
-----------

.. image:: https://live.staticflickr.com/65535/51430879732_1ecb68f409_b.jpg
    :width: 1024px
    :height: 768px

Code style
-----------

We use clang-format tool to keep the same code style for all cpp files. You can download clang-format binary from `LLVM Download Page <http://releases.llvm.org/download.html>`_
We recommend installing a plugin for your text editor or IDE which will apply clang-format tool during saving. You will need to set code style option to "FILE"

Plugins for text editors and IDEs:

- `Sublime <https://packagecontrol.io/packages/Clang%20Format>`_
- `VSCode <https://marketplace.visualstudio.com/items?itemName=xaver.clang-format>`_
- `Guide for Visual Studio <https://devblogs.microsoft.com/cppblog/clangformat-support-in-visual-studio-2017-15-7-preview-1/>`_

Unfortunately clang-format cannot handle naming, so some additional rules are:

- methods and variables should be in lower case with underscore
- class names should be in camel case
- use brackets even for single line if and for statements

For C# we use the same code style as for C++, for java there is a formatter file to take care of code style.


CI and tests
--------------

If you want to commit to the core module of BrainFlow project please check that all tests are passed, you should check CI status in your PR and fix all errors if any. Also, you are able to run failed tests locally using BrainFlow emulator.

In CI warnings as errors option is enabled for C++ code and you need to fix all of them. Also, we have `CppCheck <http://cppcheck.sourceforge.net/>`_ static analysis tool. If you see that such check failed you need to download artifact from `CppCheck Github Acttion <https://github.com/brainflow-dev/brainflow/actions?query=workflow%3ACppCheck>`_, open generated html report and fix errors. 

Pull Requests
--------------

Just try to briefly explain a goal of this PR.

Instructions to add new boards to BrainFlow
---------------------------------------------

- add new board id to `BoardIds enum in C code <https://github.com/brainflow-dev/brainflow/blob/master/src/utils/inc/brainflow_constants.h>`_ and to the same enum in all bindings
- add new object creation to `board controller C interface <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/board_controller.cpp>`_
- inherit your board from `Board class <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/inc/board.h>`_ and implement all pure virtual methods, store data in DataBuffer object, use `synthetic board <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/inc/synthetic_board.h>`_ as a reference, try to reuse code from `utils <https://github.com/brainflow-dev/brainflow/tree/master/src/utils>`_ folder
- add information about your board to `brainflow_boards.cpp <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/brainflow_boards.cpp>`_
- add new files to BOARD_CONTROLLER_SRC variable in `build.cmake <https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/build.cmake>`_, you may also need to add new directory to *target_include_directories*.

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

- build BrainFlow's core module and C++ binding in debug mode
- reproduce your issue using C++ binding
- run it with debuger and memory checker

Example for Linux, for other OSes it's similar::

    # Change build type to Debug
    python3 tools/build.py --debug --clear-build-dir --num-jobs 8
    # Create a test to reproduce your issue in C++, here we will use get_data_demo
    cd tests/cpp/get_data_demo
    mkdir build
    cd build
    cmake -DCMAKE_PREFIX_PATH=TYPE_FULL_PATH_TO_BRAINFLOW_INSTALLED_FOLDER -DCMAKE_BUILD_TYPE=Debug ..
    # e.g. cmake -DCMAKE_PREFIX_PATH=/home/andrey/brainflow/installed -DCMAKE_BUILD_TYPE=Debug ..
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

BrainFlow Emulator allows you to run all integration tests for some of supported boards without real hardware. Our CI uses it for test automation. Also, you can run it on your own PC!

Emulators are intended to test BrainFlow code for particular device. Also, some advanced emulators are capable to test very device specific features. BrainFlow users should use Synthetic board or Playback board for development.

Contributors
-------------

`Andrey1994 <https://github.com/Andrey1994>`_ is an author and maintainer of BrainFlow project. Full list of developers who can commit directly to this project and merge Pull Requests can be found `here <https://github.com/orgs/brainflow-dev/people>`_.

..  ghcontributors:: brainflow-dev/brainflow
    :limit: 25
