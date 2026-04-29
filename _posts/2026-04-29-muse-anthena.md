---
layout: post
title: BrainFlow 5.21.0
subtitle: Support for Muse S Anthena and runtime board sampling rates
tags: [release]
image: /img/MuseAnthena.png
author: andrey_parfenov
---

### Summary of Changes

* Add support for `BoardIds.MUSE_S_ANTHENA_BOARD`.
* Add `get_optical_channels` for boards which expose raw optical data.
* Add `get_board_sampling_rate`, an instance method for the actual sampling rate of a prepared board session.
* Improve `config_board` response handling in native and high level bindings.
* Add Python examples for recording all Muse S Anthena presets, recording optical data, and analyzing pulse from optical recordings.

### Muse S Anthena

We've had a lot of requests to add support for Muse S devices with Anthena firmware, and we are glad to finally deliver it in BrainFlow. If you have one of these headbands, please try the new board id and examples below, and report any issues or device-specific findings in GitHub issues or the BrainFlow Slack workspace.

Muse S devices with the newer Anthena firmware use a different BLE protocol than earlier Muse 2 and Muse S devices. BrainFlow now has a dedicated board id for it:

```python
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds

params = BrainFlowInputParams()
board = BoardShim(BoardIds.MUSE_S_ANTHENA_BOARD.value, params)
```

The board has three BrainFlow presets:

| BrainFlow preset | Data | Sampling rate | Notes |
| --- | --- | ---: | --- |
| `BrainFlowPresets.DEFAULT_PRESET` | EEG | 256 Hz | Four EEG channels `TP9,AF7,AF8,TP10` plus four additional raw EEG-like rows exposed as `other_channels`. |
| `BrainFlowPresets.AUXILIARY_PRESET` | Accelerometer and gyroscope | 52 Hz | Three accelerometer rows and three gyroscope rows. |
| `BrainFlowPresets.ANCILLARY_PRESET` | Optical data and battery | 64 Hz | Sixteen raw optical rows and one battery row. Use `get_optical_channels`, not `get_ppg_channels`, for this preset. |

The Anthena optical packets contain the data used for PPG and fNIRS-style analysis. BrainFlow exposes these values as raw optical channels instead of converting them into synthetic PPG rows inside the board parser. This keeps the data format close to the device protocol and lets users apply their own optical, pulse, or fNIRS processing.

### Muse Commands and Presets

Anthena uses BLE service `0xFE8D`, control characteristic `273e0001-4c4d-454d-96be-f03bac821358`, and data characteristics `273e0013-4c4d-454d-96be-f03bac821358` and `273e0014-4c4d-454d-96be-f03bac821358`. BrainFlow subscribes to data notifications, parses packet tags, and routes samples into the three BrainFlow presets above.

Commands are sent to the control characteristic as ASCII command strings with a length prefix and trailing newline. End users usually should not send these commands manually. Set startup options with `BrainFlowInputParams.other_info` and let BrainFlow call `prepare_session`, `start_stream`, and `stop_stream`.

During `prepare_session`, BrainFlow sends:

* `v6` to select the protocol mode.
* `s` to request status.
* `h` to stop streaming before configuration.
* the selected device preset, for example `p1041`.
* `s` again after applying the preset.

During `start_stream`, BrainFlow sends `dc001` twice and then requests status again. If low latency mode is enabled, it also sends `L1`. During `stop_stream`, BrainFlow sends `h`.

Preset selection and low latency are independent. The preset decides which data the firmware sends. `low_latency=true` decides whether BrainFlow sends `L1` during `start_stream`. For long unattended recordings, users can disable low latency if live latency matters less than a more conservative device mode.

Anthena status packets are asynchronous notifications, so `config_board` should not be treated as a reliable request/response API for these Muse commands. Use it only when you understand the firmware command you are sending.

The default startup configuration is:

```python
params.other_info = 'preset=p1041;low_latency=true'
```

You can also use shorthand syntax when you only need to select a preset:

```python
params.other_info = 'p1035'
```

The explicit format is recommended when scripts need to be clear and reproducible:

```python
params.other_info = 'preset=p1035;low_latency=false'
```

Invalid presets or invalid `low_latency` values fail early in `prepare_session` with `BrainFlowExitCodes.INVALID_ARGUMENTS_ERROR`.

The preset values accepted by BrainFlow are:

| Device preset | EEG | Optical packets | ACC/GYRO | Battery | LED behavior |
| --- | --- | --- | --- | --- | --- |
| `p20`, `p21`, `p50`, `p51`, `p60`, `p61` | EEG4 | none | yes | yes | off |
| `p1034`, `p1043` | EEG8 | Optics8 | yes | yes | bright |
| `p1044` | EEG8 | Optics8 | yes | yes | dim |
| `p1035` | EEG4 | Optics4 | yes | yes | dim |
| `p1041`, `p1042` | EEG8 | Optics16 | yes | yes | bright |
| `p1045` | EEG8 | Optics4 | yes | yes | dim |
| `p1046` | EEG8 | Optics4 | yes | yes | unknown |
| `p4129` | EEG8 | Optics4 | yes | yes | dim |

The packet tags decoded by BrainFlow are:

| Packet tag | Data type | Shape | BrainFlow preset |
| --- | --- | --- | --- |
| `0x11` | EEG4 | 4 channels, 4 samples per packet | `DEFAULT_PRESET` |
| `0x12` | EEG8 | 8 channels, 2 samples per packet | `DEFAULT_PRESET` |
| `0x34` | Optics4 | 4 channels, 3 samples per packet | `ANCILLARY_PRESET` |
| `0x35` | Optics8 | 8 channels, 2 samples per packet | `ANCILLARY_PRESET` |
| `0x36` | Optics16 | 16 channels, 1 sample per packet | `ANCILLARY_PRESET` |
| `0x47` | Accelerometer and gyroscope | 6 channels, 3 samples per packet | `AUXILIARY_PRESET` |
| `0x88`, `0x98` | Battery | 10 raw values, last parsed battery value is exposed | `ANCILLARY_PRESET` |

### Code Sample

This example records all three BrainFlow presets and writes them to separate CSV files:

```python
import time

from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds, BrainFlowPresets
from brainflow.data_filter import DataFilter


params = BrainFlowInputParams()
params.other_info = 'preset=p1041;low_latency=true'

board = BoardShim(BoardIds.MUSE_S_ANTHENA_BOARD.value, params)

try:
    board.prepare_session()
    board.start_stream()
    try:
        time.sleep(10)
        eeg_data = board.get_board_data(preset=BrainFlowPresets.DEFAULT_PRESET)
        imu_data = board.get_board_data(preset=BrainFlowPresets.AUXILIARY_PRESET)
        optical_data = board.get_board_data(preset=BrainFlowPresets.ANCILLARY_PRESET)
    finally:
        board.stop_stream()
finally:
    if board.is_prepared():
        board.release_session()

DataFilter.write_file(eeg_data, 'muse_anthena_eeg.csv', 'w')
DataFilter.write_file(imu_data, 'muse_anthena_accel_gyro.csv', 'w')
DataFilter.write_file(optical_data, 'muse_anthena_optics_battery.csv', 'w')
```

To inspect the channel layout:

```python
board_id = BoardIds.MUSE_S_ANTHENA_BOARD.value

eeg_channels = BoardShim.get_eeg_channels(board_id, BrainFlowPresets.DEFAULT_PRESET)
accel_channels = BoardShim.get_accel_channels(board_id, BrainFlowPresets.AUXILIARY_PRESET)
gyro_channels = BoardShim.get_gyro_channels(board_id, BrainFlowPresets.AUXILIARY_PRESET)
optical_channels = BoardShim.get_optical_channels(board_id, BrainFlowPresets.ANCILLARY_PRESET)
```

### Python Examples

The new Muse S Anthena examples are in [`python_package/examples/tests`](https://github.com/brainflow-dev/brainflow/tree/master/python_package/examples/tests).

[`muse_anthena_save_all.py`](https://github.com/brainflow-dev/brainflow/blob/master/python_package/examples/tests/muse_anthena_save_all.py) records EEG, IMU, and optical/battery data and writes three CSV files:

```bash
python python_package/examples/tests/muse_anthena_save_all.py --duration 30 --output-prefix muse_anthena
```

You can limit discovery with a BLE MAC address or serial number:

```bash
python python_package/examples/tests/muse_anthena_save_all.py --duration 30 --mac-address 00:11:22:33:44:55
python python_package/examples/tests/muse_anthena_save_all.py --duration 30 --serial-number MuseS-1234
```

[`muse_anthena_record_optics.py`](https://github.com/brainflow-dev/brainflow/blob/master/python_package/examples/tests/muse_anthena_record_optics.py) records only the ancillary preset, writes raw optical data, prints the expected sampling rate, estimates the timestamp-derived sampling rate, and lists active optical rows:

```bash
python python_package/examples/tests/muse_anthena_record_optics.py --duration 60 --other-info "preset=p1035;low_latency=true" --output-file muse_anthena_optics_recording.csv
```

[`muse_anthena_analyze_optics_pulse.py`](https://github.com/brainflow-dev/brainflow/blob/master/python_package/examples/tests/muse_anthena_analyze_optics_pulse.py) loads an optical CSV, detects active optical channels, filters them, combines them, estimates pulse from peaks and spectrum, and writes four PNG plots:

```bash
python python_package/examples/tests/muse_anthena_analyze_optics_pulse.py --input-file muse_anthena_optics_recording.csv --output-prefix muse_anthena_optics
```

It creates:

* `muse_anthena_optics_raw.png`
* `muse_anthena_optics_filtered.png`
* `muse_anthena_optics_pulse.png`
* `muse_anthena_optics_spectrum.png`

### `get_board_sampling_rate`

BrainFlow already had `BoardShim.get_sampling_rate(board_id, preset)`. This method is static: it reads the board description and returns the default metadata for that board and preset. That is correct for most boards, but it is not enough for boards whose sampling rate can be changed after the board object is created.

This showed up in [issue #804](https://github.com/brainflow-dev/brainflow/issues/804) for `CYTON_WIFI_BOARD`. Users can send OpenBCI sample-rate commands with `config_board`, for example `~5` for 500 Hz, but the static call still returned the default `1000` Hz. Also, command responses such as `~~` and `V` are not reliably available on the WiFi path, so callers needed a BrainFlow API for the current session state.

BrainFlow 5.21.0 adds `get_board_sampling_rate` as an instance method:

```python
from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds


params = BrainFlowInputParams()
board = BoardShim(BoardIds.CYTON_WIFI_BOARD.value, params)

try:
    board.prepare_session()
    board.config_board('~5')
    actual_sampling_rate = board.get_board_sampling_rate()
    board.start_stream()
    try:
        # read or stream data here
        pass
    finally:
        board.stop_stream()
finally:
    if board.is_prepared():
        board.release_session()
```

Use `BoardShim.get_sampling_rate(board_id, preset)` when you need static board metadata and no runtime configuration changed the sampling rate. Use `board.get_board_sampling_rate(preset)` after `prepare_session` when you need the actual sampling rate for this board instance, especially after `config_board` commands or board-specific initialization that can change it.

For Muse S Anthena, the current preset rates are the same as the static metadata, but the instance method is still useful in generic code:

```python
board.prepare_session()
eeg_rate = board.get_board_sampling_rate(BrainFlowPresets.DEFAULT_PRESET)
optical_rate = board.get_board_sampling_rate(BrainFlowPresets.ANCILLARY_PRESET)
```

This keeps code which handles both static boards and dynamically configured boards on the same API path.
