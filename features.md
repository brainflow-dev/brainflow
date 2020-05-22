---
layout: page
title: Features
subtitle: Whether you are looking for a flexible library to quickly develop research applications to analyze data from biosensors or applications for everyday life, BrainFlow caters to all needs
---

## Uniform API for all Devices

Applications on top of BrainFlow are device agnostic. 

```python
# provide device pecific info: board_id and BrainFlowInputParams
# its the only place where you provide board specific data
params = BrainFlowInputParams ()
params.ip_port = args.ip_port
params.serial_port = args.serial_port
params.mac_address = args.mac_address
params.other_info = args.other_info
params.ip_address = args.ip_address
params.ip_protocol = args.ip_protocol
params.timeout = args.timeout

board_id = args.board_id

# query device info
sampling_rate = BoardShim.get_sampling_rate (board_id)
```

## 7 Language Bindings

Deep integration into Python, C++, Java, C# and support for R, Matlab and Julia. These bindings provide exactly the same API which allows you to switch programming languages easily.

```python
board = BoardShim (board_id, params)
board.prepare_session ()

board.start_stream ()
board.start_stream ()
time.sleep (10)
data = board.get_board_data ()
board.stop_stream ()
board.release_session ()
```

## Signal Processing API

BrainFlow's signal processing API provides all required methods to process data from biosensors.

```python
eeg_channels = BoardShim.get_eeg_channels (board_id)
for channel in eeg_channels:
    DataFilter.perform_bandpass (data[channel], sampling_rate, 15.0, 6.0, 4, FilterTypes.BESSEL.value, 0)
```