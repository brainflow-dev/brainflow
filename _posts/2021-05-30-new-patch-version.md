---
layout: post
title: BrainFlow 4.2.1
subtitle: New BrainFlow Version Released
image: /img/gforcearmband.jpg
tags: [release]
author: andrey_parfenov
---


**Changes in this release:**

* change resolution of oymotion devices from 8bits to 12bits and sampling rate to 500
* change file format from csv to tsv
* add method `get_board_descr(int board_id)`
* use error description for Julia instead ID in exceptions
* add buffering for multicast streamer to increase its perf


### OYMotion

We've changed resolution to 12bits and sampling rate to 500 for all supported OYMotion devices.

<div style="text-align: center">
    <a href="https://github.com/brainflow-dev/brainflow/pull/105" title="OYMotion" target="_blank" align="center">
        <img width="484" height="430" src="https://live.staticflickr.com/65535/50760349443_368326974c_o.jpg">
    </a>
</div>

### Method `get_board_descr(int board_id)`

This method allows you to get all information about your device like:

* eeg\emg\exg channels
* timestamp channel
* sampling rate
* etc

Methods like `BoardShim.get_eeg_channels(int board_id)` are still supported and recommened to use.


```python
from pprint import pprint 

import brainflow
from brainflow.board_shim import BoardShim, BrainFlowInputParams, LogLevels, BoardIds

board_id = BoardIds.SYNTHETIC_BOARD.value
pprint(BoardShim.get_board_descr(board_id))
```

Result:

```json
{"accel_channels": [17, 18, 19],
 "battery_channel": 29,
 "ecg_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
 "eda_channels": [23],
 "eeg_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
 "eeg_names": "Fz,C3,Cz,C4,Pz,PO7,Oz,PO8,F5,F7,F3,F1,F2,F4,F6,F8",
 "emg_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
 "eog_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
 "gyro_channels": [20, 21, 22],
 "marker_channel": 31,
 "name": "Synthetic",
 "num_rows": 32,
 "package_num_channel": 0,
 "ppg_channels": [24, 25],
 "resistance_channels": [27, 28],
 "sampling_rate": 250,
 "temperature_channels": [26],
 "timestamp_channel": 30}
```