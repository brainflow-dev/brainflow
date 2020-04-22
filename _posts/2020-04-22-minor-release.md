---
layout: post
title: BrainFlow 2.5.4
subtitle: New patch version of BrainFlow
tags: [release]
---

Changes in this release:
* fix Java issue on MacOS with SIP
* add more data to Synthetic Board
* return raw data for BrainBit


#### Synthetic Board Changes

Now Synthetic Board has:

* sampling_rate: 250
* package_num_channel
* battery_channel
* timestamp_channel
* eeg_channels
* eeg_names
* emg_channels
* ecg_channels
* eog_channels
* eda_channels
* accel_channels
* gyro_channels
* eda_channels
* ppg_channels
* temperature_channels

#### BrainBit Changes

In BrainFlow we return unprocessed data wherever possible, libneurosdk provides filtered data for EEG channels(Bandpass + Bandstop) by default.

Parameters of their filters are:

* order: 4
* Bandpass: 2-30 
* Bandstop: 50
* type: Butterworth 

Now we return raw data instead. And as a user you can apply any filters by yourself and have full control.

###### Before

<p align="center">
    <img width="500" height="375" src="https://live.staticflickr.com/65535/49806784256_8e9fff6b93.jpg">
</p>

###### After

<p align="center">
    <img width="500" height="375" src="https://live.staticflickr.com/65535/49806232123_8c26d6d2b7.jpg">
</p>