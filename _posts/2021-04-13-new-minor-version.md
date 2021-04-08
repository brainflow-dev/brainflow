---
layout: post
title: BrainFlow 4.1.0
subtitle: New BrainFlow Version Released
image: /img/brainbit.png
tags: [release]
author: andrey_parfenov
---


### BrainBit

<div style="text-align: center">
    <a href="https://brainbit.com/" title="brainflow" target="_blank" align="left">
        <img width="400" height="400" src="https://live.staticflickr.com/65535/49579371806_80b1bffae1.jpg">
    </a>
</div>

So far we used SDK provided by BrainBit to work with device. This SDK is good, but it supports only Windows and MacOS.
It doesn't work on Linux OS and for devices like Raspberry Pi. We added support for all possible desktop OSes and devices like Raspberry Pi using [BLED112 dongle](https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112).

<div style="text-align: center">
    <a href="https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112" title="brainflow" target="_blank" align="left">
        <img width="300" height="238" src="https://live.staticflickr.com/65535/51101894039_262bdf73a6_o.png">
    </a>
</div>


Also, it can be useful for PCs without built in BLE support.

In terms of BrainFlow boards it is a new board with its own board id.

### For developers

Often there is a need to develop additional dynamic libraries and load them in runtime for particular board.

For example OYMotion provides only C++ library, while we need plain C interface, for BGLIB we need to wrap BGLIB callbacks by additional library and so on.

To make it easier ***DynLibBoard*** was added as a base class.

```cpp
template <int N>
class DynLibBoard : public Board
{

protected:
    volatile bool keep_alive;
    bool initialized;
    bool is_streaming;
    std::thread streaming_thread;
    std::mutex m;
    std::condition_variable cv;
    volatile int state;
    DLLLoader *dll_loader;

    virtual int call_init ();
    virtual int call_open ();
    virtual int call_close ();
    virtual int call_start ();
    virtual int call_stop ();
    virtual int call_release ();
    virtual int call_config (char *config);

    virtual void read_thread ();
    virtual std::string get_lib_name () = 0;


public:
    DynLibBoard (int board_id, struct BrainFlowInputParams params);
    virtual ~DynLibBoard ();

    virtual int prepare_session ();
    virtual int start_stream (int buffer_size, char *streamer_params);
    virtual int stop_stream ();
    virtual int release_session ();
    virtual int config_board (std::string config, std::string &response);
};
```

By default it calls the following methods from the library loaded in runtime:

```cpp
SHARED_EXPORT int CALLING_CONVENTION initialize (void *param);
SHARED_EXPORT int CALLING_CONVENTION open_device (void *param);
SHARED_EXPORT int CALLING_CONVENTION stop_stream (void *param);
SHARED_EXPORT int CALLING_CONVENTION start_stream (void *param);
SHARED_EXPORT int CALLING_CONVENTION close_device (void *param);
SHARED_EXPORT int CALLING_CONVENTION get_data (void *param);
SHARED_EXPORT int CALLING_CONVENTION release (void *param);
SHARED_EXPORT int CALLING_CONVENTION config_device (void *param);
```

You can override such methods and customize them as you need.