# BrainFlow

BrainFlow is a library intended to obtain, parse and analyze EEG, EMG, ECG and other kinds of data from biosensors.

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
