---
layout: post
title: Adding new boards to BrainFlow
image: /img/brainflow_logo.png
tags: [tutorial]
author: andrey_parfenov
---

We are trying to add as many devices as possible into BrainFlow and welcome all contributors. 

By adding your device into BrainFlow you get Python, Java, C#, Julia, Matlab, Rust, etc SDKs. Also, now you can use your board with applications and frameworks built on top of BrainFlow API.


## Steps

1. fork and clone the [repo](https://github.com/brainflow-dev/brainflow), create a branch other than master
2. add new board id to BoardIds enum in [C code](https://github.com/brainflow-dev/brainflow/blob/master/src/utils/inc/brainflow_constants.h) and to the same enum in all other bindings
3. add new object creation to [board controller C interface](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/board_controller.cpp) inside `prepare_session` method
4. add information about your board to [brainflow_boards.cpp](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/brainflow_boards.cpp)
5. inherit your board from [Board class](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/inc/board.h) and implement all pure virtual methods, store data in DataBuffer object, use [synthetic board](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/inc/synthetic_board.h) as a reference, try to reuse code from utils folder and helpers like `DynLibBoard`, `BLELibBoard`, etc
6. add new files to `BOARD_CONTROLLER_SRC` variable in [build.cmake](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/build.cmake), you may also need to add new directory to `target_include_directories`.
7. create a PR to [upstream repo](https://github.com/brainflow-dev/brainflow)
8. optional: add an emulator of your device and integrate it into [CI pipelines](https://github.com/brainflow-dev/brainflow/tree/master/.github/workflows)


### 1. Fork the repo

Create a fork of BrainFlow repo on github and run:

```
git clone %url of your fork%
git checkout -b %feature branch name%
cd brainflow
```

### 2. Update enums

In each binding there is `BoardIds` enum, you need to update them all and add new id.

For Cpp you can find it in [brainflow_constants.h](https://github.com/brainflow-dev/brainflow/blob/master/src/utils/inc/brainflow_constants.h) file. For some laguages it's located in the same file as `BoardShim` class. For Rust you need to run `cargo build --features generate_binding` to update enums

### 3. Create new board object

Go to [board_controller.cpp](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/board_controller.cpp) file and find `prepare_session` method there. This method acts as a factory,  you need to add new condition to `switch` statement there.

```cpp
case BoardIds::MUSE_2_BOARD:
	board = std::shared_ptr<Board> (new Muse (board_id, params));
	break;

```

Also, you need to include new header for your device.

### 4. Determine output channels and presets

BrainFlow returns data as 2d array, information about actual channels is static and determined by [brainflow_boards.cpp](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/brainflow_boards.cpp) file.

You need to register a new entry there and board id inside this json should match board id in `BoardIds` enum.

```cpp
/* For all real boards there are four required fields:
     *   name
     *   num_rows
     *   timestamp_channel
     *   marker_channel
     * Nice to set:
     *   package_num
     *   sampling_rate
     * Everything else is optional and up to device
    */
    brainflow_boards_json["boards"]["41"]["default"] =
    {
        {"name", "Muse2016"},
        {"sampling_rate", 256},
        {"timestamp_channel", 5},
        {"marker_channel", 6},
        {"package_num_channel", 0},
        {"num_rows", 7},
        {"eeg_channels", {1, 2, 3, 4}},
        {"eeg_names", "TP9,Fp1,Fp2,TP10"}
    };
    brainflow_boards_json["boards"]["41"]["auxiliary"] =
    {
        {"name", "Muse2016Aux"},
        {"sampling_rate", 52},
        {"timestamp_channel", 7},
        {"marker_channel", 8},
        {"package_num_channel", 0},
        {"num_rows", 9},
        {"accel_channels", {1, 2, 3}},
        {"gyro_channels", {4, 5, 6}}
    };

```

Available names for [BrainFlowPresets](https://brainflow.org/2022-07-15-brainflow-5-1-0/) are:

* default
* auxiliary
* ancillary

Default preset is required, other presets are optional.

### 5. Add new board class

You need to inherit your device from the [Board](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/inc/board.h) class and implement all pure virtual methods. 

BrainFlow has plenty of helpers for different types of IO protocols, it includes:

* serial port
* TCP/UDP sockets
* multicast and broadcasts sockets
* bluetooth
* BLE

You should reuse them as much as possible, check [utils](https://github.com/brainflow-dev/brainflow/tree/master/src/utils) folder to see all possible helpers and interfaces for them. Also, you can use one of already supported devices with the similar IO protocol as a reference. 

Good examples to use as a base are:

* Synthetic board to get familiar, it does not use IO helpers and generates data instead
* Enophone for classic bluetooth
* GanglionNative for BLE
* FreeEEG32 for serial port
* Unicorn for 3rd party binary library for IO
* Neurosity for TCP/IP 


Generic rules to keep in mind:

* initialize as much as possible inside `prepare_session` method
* send commands to start streaming and create extra threads for signal acquisition in `start_stream`
* send commands to stop streaming and stop all threads in `stop_stream`
* release all allocated resources in `release_session`

Also, base Board class has some usefull methods and fields:

* board description from step 4 will be available as a json object `board_descr`
* logging should be done via `safe_logger` method
* `prepare_for_acquisition` should be called from `start_stream` to allocate buffers and init streamers
* data should be pushed via `push_package` method from data acquisition thread running in the background
* `free_packages` should be called from `release_session`

### 6. Update Cmake files

Add new files to `BOARD_CONTROLLER_SRC` variable in [build.cmake](https://github.com/brainflow-dev/brainflow/blob/master/src/board_controller/build.cmake), you may also need to add new directory to `target_include_directories`.

### 7. Test it and send a PR

Before PR make sure that you've tested everything and configured `clang-format` tool. Check BraiinFlow docs for developers for more info about docs, codestyle, CI, etc. You will also need to add info about your device to [BrainFlow docs](https://github.com/brainflow-dev/brainflow/blob/master/docs/SupportedBoards.rst)

### 8. Emulator and CI pipelines

This step is optional but it will ensure that support for your device will not be broken in the future. You can add an [emulator of your device](https://github.com/brainflow-dev/brainflow/tree/master/emulator/brainflow_emulator) and integrate tests with emulator into [CI pipelines](https://github.com/brainflow-dev/brainflow/tree/master/.github/workflows).


If you have any questions about this process feel free to ask them in BrainFlow Slack workspace.