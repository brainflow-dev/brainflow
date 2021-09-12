<p align="center">
    <img width="400" height="160" src="https://live.staticflickr.com/65535/49908747533_f359f83610_w.jpg">
    <br>
    <img alt="GitHub all releases" src="https://img.shields.io/github/downloads/brainflow-dev/brainflow/total?color=yellow&label=Downloads%28Github%29">
    <img alt="PYPI" src="https://static.pepy.tech/personalized-badge/brainflow?period=total&units=international_system&left_color=grey&right_color=yellow&left_text=Downloads(PYPI)">
    <img alt="Nuget" src="https://img.shields.io/nuget/dt/brainflow?color=yellow&label=Downloads%28Nuget%29&logo=BrainFlow">
</p>

BrainFlow is a library intended to obtain, parse and analyze EEG, EMG, ECG and other kinds of data from biosensors.

It provides a uniform SDK to work with biosensors with primary focus on neurointerfaces, all features available for free!

#### Advantages of BrainFlow:

* powerful API with many features to simplify development
    * Straightforward API for data acquisition
    * Powerful API for signal filtering, denoising, downsampling...
    * Development tools like Synthetic board, Streaming board, logging API
* easy to use
    * BrainFlow has many bindings, you can choose programming language you like
    * All programming languages provide the same API, so it's simple to switch
    * API is uniform for all boards, it makes applications on top of BrainFlow almost board agnostic
* easy to support and extend
    * Code to read data and to perform signal processing is implemented only once in C/C++, bindings just call C/C++ methods
    * Powerful CI/CD system which runs integrations tests for each commit automatically using BrainFlow's Emulator
    * Simplified process to add new boards and methods

## [BrainFlow Docs, Dev and User guides and other information](https://brainflow.readthedocs.io)

## [BrainFlow's slack workspace](https://openbraintalk.slack.com/), use this [link to join](https://c6ber255cc.execute-api.eu-west-1.amazonaws.com/Express/)

## [For BrainFlow Developers](https://brainflow.readthedocs.io/en/master/BrainFlowDev.html)

## Build Status

Compiled with:
* MSVC on Windows
* Android NDK with Ninja
* GCC on Linux
* Clang on MacOS

*Linux and MacOS*:

[![Run Unix](https://github.com/brainflow-dev/brainflow/workflows/Run%20Unix//badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions)

*Windows*:

[![Run Unix](https://github.com/brainflow-dev/brainflow/workflows/Run%20Windows//badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions)

*Android NDK*:

[![Run Android NDK](https://github.com/brainflow-dev/brainflow/workflows/Run%20Android%20NDK//badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions)

## Brainflow Bindings

We support bindings for:
* [Python](./python-package)
* [Java](./java-package/brainflow/)
* [R](./r-package/)
* [C++](./cpp-package/)
* [C#](./csharp-package/brainflow/)
* [Matlab](./matlab-package/brainflow)
* [Julia](.julia-package/brainflow)

## Partners and Sponsors

[![OpenBCI](https://live.staticflickr.com/65535/49913349191_0cbd41157c_w.jpg)](https://openbci.com/)

## License: 
MIT
