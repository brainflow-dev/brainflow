---
layout: post
title: BrainFlow 4.7.0
subtitle: Built-in BLE library, Muse devices wo dongles
image: /img/ble.png
tags: [release]
author: andrey_parfenov
---


### Summary

* New helper library for BLE devices
* Muse 2 and Muse S work without dongle
* Fix `set_log_file` issue for C++ API - [#359](https://github.com/brainflow-dev/brainflow/issues/359)
* Fix start, stop, repeat issue for BrainBit and Callibri - [#363](https://github.com/brainflow-dev/brainflow/issues/363)

### SimpleBLE library 

We've integrated [SimpleBLE library](https://github.com/OpenBluetoothToolbox/SimpleBLE) into BrainFlow. Thanks [Kevin Dewald](https://github.com/kdewald) for implementing cross platform library to work with BLE protocol!

To use it inside BrainFlow you need to inherit your device from `BLELibBoard` and build BrainFlow with `--ble` flag, if you use `build.py` or provide `-DBUILD_BLE=ON` if you use CMake. It works for MacOS 10.15+ and Windows 10.0.19041+, for Linux you need to install packages `libdbus-1-dev` for Ubuntu and `dbus-devel` for CentOS. For Linux we also recommend to compile BrainFlow from the source.

For examples you can refer to implementation for Muse devices.

As of right now, we have no plans to remove code for old BrainFlow boards which uses BLED112 dongle, but SimpleBLE library will be a standard to work with BLE for upcoming devices.

### Muse devices without BLED112 dongle

<div style="text-align: center">
    <a href="https://choosemuse.com/muse-s/" title="MuseS" target="_blank" align="center">
        <img width="500" height="500" src="https://live.staticflickr.com/65535/51249005962_026502fee0.jpg">
    </a>
</div>

We've used this library to add support for Muse devices without dongle, old boards(`MUSE_2_BLED_BOARD`  and `MUSE_S_BLED_BOARD`) are still available and still work via BLED112 dongle. Here we've added new BrainFlow boards `MUSE_2_BOARD` and `MUSE_S_BOARD`, they work via native API and require no dongle. 

Feel free to test them and report [issues](https://github.com/brainflow-dev/brainflow/issues) if any!
