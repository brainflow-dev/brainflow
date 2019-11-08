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

Also you can run integration tests manually for any board even if you dont have real hardware, check BrainFlow Emulator page for details.

Pull Requests
--------------

Just try to briefly explain a goal of this PR

Instructions to add new boards to BrainFlow
---------------------------------------------

- add new object creation to board_controller.cpp
- add new board Id to BoardIds enum in C++ code and in all bindings
- inherit your board from Board class and implement all pure virtual methods, store data in DataBuffer object, use synthetic board as a reference, try to reuse code from utils folder
- add information about your board to brainflow_boards.json
- add new files to CmakeLists.txt

**You've just written Python, Java, C#, R, C++ ... SDKs for your board! Also now you can use your new board with applications and frameworks which use BrainFlow API.**

To enable automation testing you should develop a simple emulator for your new board and add tests to .travis.yml and appveyour.yml, also make sure that all current tests are passed and feel free to send a PR.
