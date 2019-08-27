# Brainflow

## [Documentation, Contributing guide and other information](https://brainflow.readthedocs.io/en/latest/index.html)

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG data from OpenBCI boards.

Core module of this library is implemented in C\C++ and available for all bindings as a dynamic library. This approach allows to reduce code duplication and simplify maintenance, also it's better than any streaming based solution because using this library you don't need to worry about inter process communication and it's a single dependency.


## *NOTE: Brainflow was forked by OpenBCI community, I will contribute to [OpenBCI's fork](https://github.com/OpenBCI/brainflow) and maintain it. To download the latest stable version of Brainflow feel free to use OpenBCI's fork. Both versions are exactly the same*


## [BrainFlow Docs, Dev and User guides and other information](https://brainflow.readthedocs.io/en/latest/index.html)

## Build status
*Linux(Travis)*:

[![Build Status](https://travis-ci.com/Andrey1994/brainflow.svg?branch=master)](https://travis-ci.com/Andrey1994/brainflow)

*Windows(AppVeyour)*:

[![Build status](https://ci.appveyor.com/api/projects/status/4gr8uy65f86eh2b5?svg=true)](https://ci.appveyor.com/project/Andrey1994/brainflow)

## Brainflow Bindings
We support bindings for:
* [python](./python-package)
* [java](./java-package/brainflow/)
* [R](./r-package/)
* [CPP](./cpp-package/)
* [Matlab](./matlab-package/brainflow/)
* [C# (Windows only)](./csharp-package/brainflow/)

These bindings just call methods from dynamic libraries, if you are interested in other programming languages - feel free to create feature request

## License: 
MIT
