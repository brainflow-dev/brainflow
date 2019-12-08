# Brainflow

BrainFlow is a library intended to obtain, parse and analyze EEG, EMG, ECG and other kinds of data from BCI boards.

It provides a **uniform API for all supported boards**, it means that you can switch boards without any changes in code, also if you add your board to BrainFlow it will work with almost all applications built on top on BrainFlow from the box. Also there is **powerfull API to perform signal processing** which you can use even without BCI headset.

## [BrainFlow Docs, Dev and User guides and other information](https://brainflow.readthedocs.io)

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
