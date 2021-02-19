---
layout: post
title: BrainFlow vs LSL
image: /img/lsl.jpeg
author: andrey_parfenov
---


<div style="text-align: center">
    <a href="https://github.com/brainflow-dev/brainflow" title="lsl" target="_blank" align="left">
        <img width="260" height="194" src="https://live.staticflickr.com/65535/50910162901_86602ce394_o.jpg">
    </a>
</div>


### What is LSL? 

The lab streaming layer (LSL) is a system for the unified collection of measurement time series in research experiments that handles both the networking, time-synchronization, (near-) real-time access as well as optionally the centralized collection, viewing and disk recording of the data.


### Comparison of BrainFlow and LSL

LSL works on top of TCP\IP stack, so it means that devices, which use BT, BLE or serial port for communication can not stream LSL data directly. Also, many boards use Arduino which is not supported by LSL.

Because of this in most cases, a workflow that uses LSL is as follows:

1. The device sends data via a serial port, Bluetooth Low Energy (BLE), a socket, or other. 
2. The researcher develops an application that converts the device data to an LSL stream. Examples of this include the OpenBCI GUI Networking Widget, and Muse-LSL.
3. The output LSL streams can be read by other applications to perform data analysis.

Such pipeline works fine for researchers, but not ideal for software engineers, because it's difficult to automate it, not perfect from UX point of view and requires sophisticated Inter Process Communication.

Additionally, LSL doesn’t allow sending/receiving commands to/from a device, which means its state cannot be changed during streaming. This lack of communication with the device can also cause problems with timestamp synchronization, since LSL has no information about the transmission period from the device to the LSL application.

To overcome these limitations, BrainFlow allows sending/receiving commands to a device through a serial port, BLE, sockets, and more, because it's not attached to particular communication protocol. Also, BrainFlow works directly with device and doesn't require additional applications.


***LSL was originally designed as a tool to synchronize data streams. It is not intended to be a replacement for device-specific SDKs. In contrast, BrainFlow is designed to be a device agnostic SDK for a growing list of supported devices, which standardizes the data stream at the software level.***
