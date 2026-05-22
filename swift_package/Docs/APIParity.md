# Swift API Parity

Swift mirrors the public Python and Java API shape, with Swift-native signatures where required by the language.

## BoardShim

Implemented:

- Session lifecycle: `prepare_session`, `start_stream`, `stop_stream`, `release_session`, `release_all_sessions`, `is_prepared`.
- Data access: `get_current_board_data`, `get_board_data`, `get_board_data_count`, `get_board_id`, `get_board_sampling_rate`, `insert_marker`.
- Stream/config: `add_streamer`, `delete_streamer`, `config_board`, `config_board_with_bytes`.
- Metadata: sampling rate, package/timestamp/marker/battery rows, row count, EEG names, board presets, board description, device name, all channel getters exposed by the C ABI.
- Logging/version: board logger controls, log file, log message, version.

## DataFilter

Implemented:

- Filters/noise/detrend: lowpass, highpass, bandpass, bandstop, environmental noise removal, rolling filter, detrend.
- Transforms/features: downsampling, wavelet transform/inverse/denoising, CSP, windowing, FFT/IFFT, PSD/Welch, band powers, ICA.
- Helpers: stddev, railed percentage, oxygen level, heart rate, peak detection, nearest power of two, file IO, reshape helpers, logging, version.

Swift differs from Java/Python for in-place operations by using `inout [Double]`.

## MLModel

Implemented:

- `BrainFlowModelParams`
- `prepare`
- `release`
- `predict`
- logger controls
- `release_all`
- `get_version`

## Known Packaging Notes

- Runtime calls require native BrainFlow libraries to be available through `BRAINFLOW_LIB_DIR`, system loader paths, `installed/lib`, or app bundle resources.
- iOS execution depends on shipping BrainFlow native binaries compiled for iOS. The Swift API compiles for iOS, but native libraries still determine runtime support.
