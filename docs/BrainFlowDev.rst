BrainFlow Dev
=====================

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

We try to keep the same code style for all bindings as well, even if it doesn't match PEP or other standards

For example we add spaces before and after assignment operator to specify default value for method's param and add spaces before brackets

CI and tests
-------------

If you want to commit to core module of brainflow project please check that all tests are passed, you can enable `Travis CI <https://travis-ci.com/>`_ and `AppVeyour <https://ci.appveyor.com>`_ for your fork of BrainFlow to run tests automatically, or check CI status directly in your PR

Pull Requests
--------------

Just try to briefly explain a goal of this PR

