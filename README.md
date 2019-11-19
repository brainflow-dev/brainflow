# Brainflow

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG data from OpenBCI boards.

Core module of this library is implemented in C\C++ and available for all bindings as a dynamic library. This approach allows to reduce code duplication and simplify maintenance, also it's better than any streaming based solution because using this library you don't need to worry about inter process communication and it's a single dependency.

This project was forked by OpenBCI from [Andrey1994/brainflow](https://github.com/Andrey1994/brainflow)

## [BrainFlow Docs, Dev and User guides and other information](https://brainflow.readthedocs.io/)

## Build status
Compiled with:
* MSVC on Windows
* GCC on Linux
* Clang on MacOS

*Linux and MacOS(Travis)*:

[![Build Status](https://travis-ci.com/OpenBCI/brainflow.svg?branch=master)](https://travis-ci.com/OpenBCI/brainflow)

*Windows(AppVeyour)*:

[![Build status](https://ci.appveyor.com/api/projects/status/kuuoa32me3253jad/branch/master?svg=true)](https://ci.appveyor.com/project/daniellasry/brainflow/branch/master)

## Brainflow Bindings
We support bindings for:
* [python](./python-package)
* [java](./java-package/brainflow/)
* [R](./r-package/)
* [C++](./cpp-package/)
* [C# (Windows only)](./csharp-package/brainflow/)

## License: 
MIT
