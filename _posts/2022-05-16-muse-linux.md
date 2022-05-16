---
layout: post
title: BrainFlow 4.9.3
subtitle: Muse devices on Linux
image: /img/Muse-2-001.jpg
tags: [release]
author: andrey_parfenov
---


### Summary

* Update SimpleBLE library
* Fix `terminate called without active exception` for native BLE boards on Linux - [#434](https://github.com/brainflow-dev/brainflow/issues/434)

### Fixes for Muse boards

<div style="text-align: center">
    <a href="https://choosemuse.com/muse-s/" title="MuseS" target="_blank" align="center">
        <img width="500" height="500" src="https://live.staticflickr.com/65535/51249005962_026502fee0.jpg">
    </a>
</div>

We've updated [SimpleBLE library](https://github.com/OpenBluetoothToolbox/SimpleBLE) inside BrainFlow and it should fix some issues with Muse devices on Linux and MacOS.

If you had issues like on the snippet below you should update BrainFlow version to 4.9.3.

```
[2022-05-07 22:54:49.125] [board_logger] [info] incoming json: {
    "file": "",
    "ip_address": "",
    "ip_port": 0,
    "ip_protocol": 0,
    "mac_address": "00:55:DA:B7:86:B5",
    "other_info": "",
    "serial_number": "",
    "serial_port": "",
    "timeout": 0
}
[2022-05-07 22:54:49.126] [board_logger] [trace] Board object created 38
[2022-05-07 22:54:49.126] [board_logger] [info] Use timeout for discovery: 5
[2022-05-07 22:54:49.126] [board_logger] [debug] use dyn lib: /home/username/projects/realtime_EEG/muse_venv/lib/python3.8/site-packages/brainflow/lib/libsimpleble-c.so
[2022-05-07 22:54:49.130] [board_logger] [info] found 2 BLE adapter(s)
[2022-05-07 22:54:49.733] [board_logger] [trace] address C8:CD:41:44:81:5C
[2022-05-07 22:54:49.733] [board_logger] [trace] identifier LE_WH-1000XM4
[2022-05-07 22:54:50.175] [board_logger] [trace] address 51:8D:0C:19:B4:E9
[2022-05-07 22:54:50.175] [board_logger] [trace] identifier 
[2022-05-07 22:54:50.644] [board_logger] [trace] address 00:55:DA:B7:86:B5
[2022-05-07 22:54:50.644] [board_logger] [trace] identifier Muse-86B5
[2022-05-07 22:54:50.644] [board_logger] [info] Found Muse device
[2022-05-07 22:54:51.520] [board_logger] [info] Connected to Muse Device
[2022-05-07 22:54:52.522] [board_logger] [trace] found servce 00001801-0000-1000-8000-00805f9b34fb
[2022-05-07 22:54:52.522] [board_logger] [trace] found characteristic 00002a05-0000-1000-8000-00805f9b34fb
[2022-05-07 22:54:52.523] [board_logger] [trace] found servce 0000fe8d-0000-1000-8000-00805f9b34fb
[2022-05-07 22:54:52.523] [board_logger] [trace] found characteristic 273e0001-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.523] [board_logger] [info] found control characteristic
[2022-05-07 22:54:52.523] [board_logger] [trace] found characteristic 273e0008-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.523] [board_logger] [trace] found characteristic 273e0009-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.524] [board_logger] [trace] found characteristic 273e000a-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.525] [board_logger] [trace] found characteristic 273e000b-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.525] [board_logger] [trace] found characteristic 273e0002-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.525] [board_logger] [trace] found characteristic 273e0003-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.525] [board_logger] [trace] found characteristic 273e0004-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.526] [board_logger] [trace] found characteristic 273e0005-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.526] [board_logger] [trace] found characteristic 273e0006-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.527] [board_logger] [trace] found characteristic 273e0007-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.527] [board_logger] [trace] found characteristic 273e000c-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.527] [board_logger] [trace] found characteristic 273e000d-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.527] [board_logger] [trace] found characteristic 273e000e-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.527] [board_logger] [trace] found characteristic 273e000f-4c4d-454d-96be-f03bac821358
[2022-05-07 22:54:52.528] [board_logger] [trace] found characteristic 273e0010-4c4d-454d-96be-f03bac821358
terminate called without an active exception

```
