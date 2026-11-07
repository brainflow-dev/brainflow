.. _swift-api-parity-label:

Swift API Parity
================

Swift mirrors the public Python and Java API shape, with Swift-native signatures where required by
the language.

For native library setup and iOS/macOS app integration, see :ref:`swift-installation-label`.

BoardShim
----------

Implemented:

- Session lifecycle: :code:`prepare_session`, :code:`start_stream`, :code:`stop_stream`,
  :code:`release_session`, :code:`release_all_sessions`, and :code:`is_prepared`.
- Data access: :code:`get_current_board_data`, :code:`get_board_data`,
  :code:`get_board_data_count`, :code:`get_board_id`, :code:`get_board_sampling_rate`, and
  :code:`insert_marker`.
- Stream and config methods: :code:`add_streamer`, :code:`delete_streamer`,
  :code:`config_board`, and :code:`config_board_with_bytes`.
- Metadata: sampling rate, package/timestamp/marker/battery rows, row count, EEG names, board
  presets, board description, device name, and all channel getters exposed by the C ABI.
- Logging and version APIs: board logger controls, log file, log message, and version.

DataFilter
----------

Implemented:

- Filters, noise removal, and detrending: lowpass, highpass, bandpass, bandstop, environmental
  noise removal, rolling filter, and detrend.
- Transforms and features: downsampling, wavelet transform/inverse/denoising, CSP, windowing,
  FFT/IFFT, PSD/Welch, band powers, and ICA.
- Signal quality checks and band powers over accepted channels use :code:`SignalQualityOptions`.
  See :ref:`quality-aware-band-powers-label` for the result and policy fields.
- Helpers: standard deviation, railed percentage, oxygen level, heart rate, peak detection,
  nearest power of two, file I/O, reshape helpers, logging, and version.

Swift differs from Java and Python for in-place operations by using :code:`inout [Double]`.

MLModel
-------

Implemented:

- :code:`BrainFlowModelParams`
- :code:`prepare`
- :code:`release`
- :code:`predict`
- Logger controls
- :code:`release_all`
- :code:`get_version`
