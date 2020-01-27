BrainFlow Dev
===============

Code style
-----------

We use clang-format tool to keep the same code style for all cpp files. You can download clang-format binary from `LLVM Download Page <http://releases.llvm.org/download.html>`_
We recommend to install a plugin for your text editor or IDE which will apply clang-format tool during saving. You will need to set code style option to "FILE"

Plugins for text editors and IDEs:

- `Sublime <https://packagecontrol.io/packages/Clang%20Format>`_
- `VSCode <https://marketplace.visualstudio.com/items?itemName=xaver.clang-format>`_
-  clang-format tool is preinstalled for Visual Studio


Unfortunately clang-format can not handle naming, so some additional rules are:
- methods and variables should be in lower case with underscore
- class names should be in camel case
- use brackets even for single line if and for statements

We try to keep the same code style for all bindings as well, even if it doesn't match PEP or other standards. For example we add spaces before and after assignment operator to specify default value for method's params and add spaces before brackets.

CI and tests
--------------

If you want to commit to core module of brainflow project please check that all tests are passed, you can enable `Travis CI <https://travis-ci.com/>`_ and `AppVeyour <https://ci.appveyor.com>`_ for your fork of BrainFlow to run tests automatically, or check CI status directly in your PR.

Also you can run integration tests manually for any board even if you dont have real hardware, check :ref:`emulator-label` page for details.

Pull Requests
--------------

Just try to briefly explain a goal of this PR.

Instructions to add new boards to BrainFlow
---------------------------------------------

- add new object creation to board_controller.cpp
- add new board Id to BoardIds enum in C++ code and in all bindings
- inherit your board from Board class and implement all pure virtual methods, store data in DataBuffer object, use synthetic board as a reference, try to reuse code from utils folder
- add information about your board to brainflow_boards.json
- add new files to CmakeLists.txt

**You've just written Python, Java, C#, R, C++ ... SDKs for your board! Also now you can use your new board with applications and frameworks which use BrainFlow API.**

To enable automation testing you should develop a simple emulator for your new board and add tests to .travis.yml and appveyour.yml, also make sure that all current tests are passed and feel free to send a PR.

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
    cmake -DCMAKE_PREFIX_PATH=TYPE_FULL_PATH_TO_BRAINFLOW_INSTALLED_FOLDER ..
    # e.g. cmake -DCMAKE_PREFIX_PATH=/home/andrey/brainflow/installed_linux -DCMAKE_BUILD_TYPE=Debug ..
    make
    # Run Valgrind to check memory errors
    # Here we use command line for Ganglion
    sudo valgrind --error-exitcode=1 --leak-check=full ./brainflow_get_data --board-id 1 --serial-port /dev/ttyACM0 --mac-address e6:73:73:18:09:b1
    # Valgrind will print Error Summary and exact line numbers
    # Run gdb and get backtrace
    sudo gdb --args ./brainflow_get_data --board-id 1 --serial-port /dev/ttyACM0 --mac-address e6:73:73:18:09:b1
    # In gdb terminal type 'r' to run the program and as soon as error occurs, type 'bt' to see backtrace with exact lines of code and call stack
