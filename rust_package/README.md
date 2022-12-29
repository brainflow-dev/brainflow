<p align="center">
    <img width="400" height="160" src="https://live.staticflickr.com/65535/49908747533_f359f83610_w.jpg">
    <br>
    <a href="https://github.com/brainflow-dev/brainflow/releases">
       <img alt="GitHub all releases" src="https://img.shields.io/github/downloads/brainflow-dev/brainflow/total?color=yellow&label=Downloads%28Github%29">
    </a>
    <a href="https://pypi.org/project/brainflow/">
        <img alt="PYPI" src="https://static.pepy.tech/personalized-badge/brainflow?period=total&units=international_system&left_color=grey&right_color=yellow&left_text=Downloads(PYPI)">
    </a>
    <a href="https://www.nuget.org/packages/brainflow/">
        <img alt="Nuget" src="https://img.shields.io/nuget/dt/brainflow?color=yellow&label=Downloads%28Nuget%29&logo=BrainFlow">
    </a>
</p>

BrainFlow is a library intended to obtain, parse and analyze EEG, EMG, ECG, and other kinds of data from biosensors.

It provides a uniform SDK to work with biosensors with a primary focus on neurointerfaces, all features available for free and distributed under MIT license.

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

## Resources

* [***BrainFlow Docs, Dev and User guides and other information***](https://brainflow.readthedocs.io)
* [***BrainFlow's slack workspace***](https://openbraintalk.slack.com/)***, use this*** [***link to join***](https://c6ber255cc.execute-api.eu-west-1.amazonaws.com/Express/)
* [***For BrainFlow Developers***](https://brainflow.readthedocs.io/en/master/BrainFlowDev.html)

## Contribution guidelines

If you want to contribute to BrainFlow, be sure to review the [contribution guidelines](https://brainflow.readthedocs.io/en/stable/BrainFlowDev.html). This project adheres to [BrainFlow's code of conduct](https://github.com/brainflow-dev/brainflow/blob/master/CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code.

We use [GitHub issues](https://github.com/brainflow-dev/brainflow/issues) for tracking requests and bugs, please use BrainFlow's slack for general discussions.

The BrainFlow project strives to abide by generally accepted best practices in open-source software development.

## Build Status
|          Build Type         	|                                                                                Status                                                                               	|
|:---------------------------:	|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------:	|
|        Windows Tests        	|   [![Run Windows](https://github.com/brainflow-dev/brainflow/actions/workflows/run_windows.yml/badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions/workflows/run_windows.yml)  	|
| Unix(Linix and MacOS) Tests 	| [![Run Unix](https://github.com/brainflow-dev/brainflow/actions/workflows/run_unix.yml/badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions/workflows/run_unix.yml) 	|
|        Android Tests        	|   [![Run Android NDK](https://github.com/brainflow-dev/brainflow/actions/workflows/run_android.yml/badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions/workflows/run_android.yml)   	|
|         Alpine Tests        	|       [![Run Alpine](https://github.com/brainflow-dev/brainflow/actions/workflows/run_alpine.yml/badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions/workflows/run_alpine.yml)       	|
|        Valgrind Tests       	|     [![Run Valgrind](https://github.com/brainflow-dev/brainflow/actions/workflows/valgrind.yml/badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions/workflows/valgrind.yml)      	|
|           CppCheck          	|    [![CppCheck](https://github.com/brainflow-dev/brainflow/actions/workflows/cppcheck.yml/badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions/workflows/cppcheck.yml)   	|
|         Clang-Format        	|   [![Clang Format](https://github.com/brainflow-dev/brainflow/actions/workflows/clang_format.yml/badge.svg?branch=master)](https://github.com/brainflow-dev/brainflow/actions/workflows/clang_format.yml)   	|  

## Brainflow Bindings

We support bindings for:
* [Python](./python_package)
* [Java](./java_package/brainflow/)
* [R](./r_package/)
* [C++](./cpp_package/)
* [C#](./csharp_package/brainflow/)
* [Matlab](./matlab_package/brainflow)
* [Julia](./julia_package/brainflow)

## Partners and Sponsors

[![OpenBCI](https://live.staticflickr.com/65535/51618456586_7b533781e2.jpg)](https://openbci.com/)

## License: 
[MIT](https://github.com/brainflow-dev/brainflow/blob/master/LICENSE)
