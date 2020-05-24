---
layout: post
title: BrainFlow 3.0.0
subtitle: New Major Release
image: /img/callibri.jpg
tags: [release]
---

### Changes in this release:

* Add Callibri device
* Add serial_number field to BrainFlowInputParams structure


#### Callibri

Callibri – a wearable device embedded with multiple sensors to monitor physiological data spanning across muscle movement, breathing & heart rates, and more.

There are several versions of Callibri device, we've added support for yellow device.

**In terms of BrainFlow's boards it's represented as three different devices: CallibriEEG, CallibriEMG, CallibriECG. It's the same device but with different presets for EEG,EMG and ECG respectively.**

#### Serial Number field

It's a breaking change and the reason why we released new major version. Before you were able to specify serial number for BrainBit and Unicorn in other_info field, now it should be in serial_number field.