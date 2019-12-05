# BrainFlow

## [Documentation, Contributing guide and other information](https://brainflow.readthedocs.io/)

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG... data from any BCI board.

It provides a **uniform API for all supported boards**, it means that you can switch boards without any changes in code, also if you add your board to BrainFlow it will work with almost all applications built on top on BrainFlow from the box. Also there is **powerfull API to perform signal processing** which you can use even without BCI headset.

## Build status

Compiled with:
* MSVC on Windows
* GCC on Linux
* Clang on MacOS

*Linux and MacOS(Travis)*:

[![Build Status](https://travis-ci.com/Andrey1994/brainflow.svg?branch=master)](https://travis-ci.com/Andrey1994/brainflow)

*Windows(AppVeyour)*:

[![Build status](https://ci.appveyor.com/api/projects/status/4gr8uy65f86eh2b5/branch/master?svg=true)](https://ci.appveyor.com/project/Andrey1994/brainflow/branch/master)

## Extend BrainFLow SDK

*We will be glad to add new boards, signal processing methods and new features to BrainFlow SDK.*
*For more information about process of adding new features to BrainFlow read* [this](https://brainflow.readthedocs.io/en/stable/BrainFlowDev.html)

*Also we can help you to integrate BrainFlow SDK to your apps, fromeworks etc.*

## Brainflow Bindings
We support bindings for:
* [python](./python-package)
* [java](./java-package/brainflow/)
* [R](./r-package/)
* [C++](./cpp-package/)
* [C# (Windows only)](./csharp-package/brainflow/)

## License: 
MIT
