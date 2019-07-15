# Brainflow

## [Documentation, Contributing guide and other information](https://brainflow.readthedocs.io/en/latest/index.html)

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG data from OpenBCI boards.

Core module of this library is implemented in C\C++ and available for all bindings as a dynamic library. This approach allows to reduce code duplication and simplify maintenance, also it's better than any streaming based solution because using this library you don't need to worry about inter process communication and it's a single dependency.


## *NOTE: Brainflow was forked by OpenBCI community, I will contribute to [OpenBCI's fork](https://github.com/OpenBCI/brainflow) and maintain it. To download the latest stable version of Brainflow feel free to use OpenBCI's fork. Both versions are exactly the same*


## Build status
*Linux(Travis)*:

[![Build Status](https://travis-ci.org/Andrey1994/brainflow.svg?branch=master)](https://travis-ci.org/Andrey1994/brainflow)

*Windows(AppVeyour)*:

[![Build status](https://ci.appveyor.com/api/projects/status/4gr8uy65f86eh2b5?svg=true)](https://ci.appveyor.com/project/Andrey1994/brainflow)

## License: 
MIT
