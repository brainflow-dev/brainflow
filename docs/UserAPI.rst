User API
==========

BrainFlow User API has three main modules:

- BoardShim to read data from a board, it calls methods from underlying BoardController library
- DataFilter to perform signal processing, it calls methods from underlying DataHandler library
- MLModel to calculate derivative metrics, it calls methods from underlying MLModule library

These classes are independent, so if you want, you can use BrainFlow API only for data streaming and perform signal processing by yourself and vice versa.

BrainFlow data acqusition API is board agnostic, so **to select a specific board you need to pass BrainFlow's board id to BoardShim's constructor and an instance of BrainFlowInputParams structure** which should hold information for your specific board, check :ref:`supported-boards-label`. for details.
This abstraction allows you to switch boards without any changes in code.

In BoardShim, all board data is returned as a 2d array. Rows in this array may contain timestamps, EEG and EMG data and so on. To see instructions how to query specific kind of data check :ref:`data-format-label` and :ref:`code-samples-label`.

Band power preprocessing and data length
----------------------------------------

``get_avg_band_powers`` uses bands 2-4, 4-8, 8-13, 13-30, and 30-45 Hz and delegates
to ``get_custom_band_powers``. The following behavior is shared by all bindings.

With filtering enabled, each selected channel is demeaned. Fourth-order Butterworth
48-52 and 58-62 Hz mains notches are applied when their upper edge is below 90 percent
of Nyquist, avoiding poorly conditioned near-Nyquist designs. These notches attenuate
custom bands that overlap their ranges. Preprocessing does not depend on the requested
integration bands, and no passband is inferred from those bands. Filtering uses forward
and backward passes with odd reflection padding and independently initialized states.

After filtering, a guard of ``G`` real samples is discarded from each end before the
Welch PSD is calculated. The same ``G`` is used for padding. It is estimated from the
complete cascade's impulse response: the absolute tail beyond ``G`` contains at most
0.1 percent of its absolute sum, over a simulation horizon derived from the poles.
This estimates transient settling; it does not guarantee removal of all edge effects.
Provide extra surrounding samples to retain the desired analysis interval. In live use,
the most recent retained sample is ``G / sampling_rate`` seconds behind the newest input.

Welch uses a Hann window, 80 percent overlap, and
``nfft = max(8, 2 * get_nearest_power_of_two(sampling_rate))``. Inputs must contain at least
``nfft + 2 * G`` samples per selected channel when filtering is enabled. Short inputs
raise an error instead of silently lowering the frequency resolution. With filtering
disabled, there is no demeaning, filtering, or trimming, and at least ``nfft`` samples
are required. The native and Python ``get_custom_band_powers_with_options`` APIs allow
explicit low/high cutoffs, mains selection, FFT size, and whole-record detrending.
Cutoff zero disables that passband edge. Mains values are -1 for automatic selection,
0 for none, 1 for 50 Hz, 2 for 60 Hz, and 3 for both. Explicit notches must fit below
Nyquist. Detrending in this API is independent of the filtering flag; select
``NO_DETREND`` to preserve DC. Welch does not detrend individual segments.

Python ``get_band_power_info`` (native ``get_band_power_settings``) reports the FFT size
and edge margin before collection. Python also reports minimum samples, bin spacing,
margin in seconds, and optionally the retained half-open interval. Bin spacing is not
the same as the resolving bandwidth of a window. Other bindings may preprocess an
extended buffer externally and call their existing method with filtering disabled.

Selected samples and band edges must be finite. Each band must satisfy
``0 <= start < stop <= sampling_rate / 2``. Power is integrated over the exact requested
frequency bounds, including partial intervals at band edges.

The first output contains absolute band powers averaged across selected channels, then
normalized by the sum of those means over the requested bands. The second output is
historically named ``stddev`` but contains the coefficient of variation: population
standard deviation divided by mean absolute power across channels for each band. A
zero-power band has zero variation; an all-zero total returns zero normalized powers.
Overlapping bands contribute separately to the normalization sum.

These preprocessing and integration changes affect extracted features. Classifiers trained
with earlier band power behavior should be revalidated before using the new features.

Signal-processing conventions and validation
--------------------------------------------

PSD and Welch return one-sided power density in input-units squared per Hz, normalized
by ``sampling_rate * sum(window**2)``. DC and Nyquist are not doubled. This corrects
the historical underestimation with nonrectangular windows and integer overflow for
large FFTs. Absolute powers therefore change. Windows are periodic; the four-term
Blackman-Harris coefficients are 0.355768, 0.487396, 0.144232, and 0.012604.

Public filters require positive finite cutoffs strictly below Nyquist, ascending
band edges, order 1-8, and positive finite Chebyshev ripple. Zero-phase methods
preserve length and use reflected padding, limited to the available samples, with
independent steady-state initialization in each direction. Short inputs can still
have substantial boundary effects. Causal calls start from zero state on each call.
Native/Python ``get_filter_settling_samples`` reports the estimated margin for a
specified design; exclude it at both ends for zero-phase analysis or at startup
for a causal filter. Filter kinds are 0 lowpass, 1 highpass, 2 bandpass, 3 bandstop;
``low_cutoff`` holds the single cutoff for either lowpass or highpass.
The native streaming-filter API and Python ``StreamingFilter`` retain causal SOS
states across chunks, with explicit reset and release/close operations. Chunked
processing therefore matches a single causal call over the concatenated samples.
Combined mains removal is equivalent to the separate 50 Hz and 60 Hz zero-phase calls.
Nonfinite input is rejected before in-place filtering, detrending, or aggregation.

Linear detrending subtracts the least-squares line; a one-sample input becomes zero.
Standard deviations use the population convention. Rolling mean and median both
use the available prefix until the window fills. Downsampling remains a block
aggregator: it drops an incomplete trailing block and does not apply an anti-alias
filter. ``EACH`` selects the last sample in each complete block.
The separate native/Python ``perform_decimation`` applies a centered Hamming-windowed
sinc FIR before selecting samples. Its cutoff is 80 percent of the new Nyquist,
length is ``20 * factor + 1``, endpoints use reflection, and the output contains
``floor(input_length / factor)`` samples aligned to input indices ``0, factor, ...``.
It is an offline operation; endpoint effects remain possible.

Inverse-wavelet bindings validate metadata and actual buffer sizes through the new
checked native entry point. The original C API and C++ pointer overload remain for
compatibility; callers of these interfaces must supply correctly sized allocations.
Use the checked C API or C++ vector overload when sizes are available.

CSP requires finite data, exact binary labels, both classes, and full-rank composite
covariance. ICA rejects insufficient retained rank and reports nonconvergence as
``GENERAL_ERROR``. Its mixing matrix is channels by components and reconstructs
centered input as ``A @ S``. The options API exposes iteration limit, tolerance, and
random seed; successful return means convergence was reached.

Heart-rate estimation detrends both channels before Welch and requires a shared
distinct peak between 35 and 230 BPM. Each channel's peak must exceed four times
the in-band median; its three-bin neighborhood must contain at least 45 percent
of in-band power, and channel peaks must agree within one bin. FFT size must be
a power of two, at least 1024, with bin spacing no greater than 0.25 Hz. Oxygen
estimation uses a second-order 0.5-4 Hz zero-phase bandpass, excludes response-based
guards at both ends, requires at least four seconds retained, and checks positive
DC/AC and a distinct shared pulse. Existing calibration coefficients remain explicit
arguments. Invalid or insufficient pulse signals raise ``INVALID_ARGUMENTS_ERROR``;
insufficient data for oxygen's retained interval raises ``INVALID_BUFFER_SIZE_ERROR``.
These are numerical quality checks, not physiological validation.

``get_railed_percentage`` preserves the legacy 4.5 V, 24-bit ADC peak/full-scale
metric with microvolt input and gain, including 100 percent for flat signals; its
result is now bounded to 0-100. Native/Python ``get_clipping_percentage`` instead
counts samples at or beyond caller-provided lower/upper limits.
``get_flatline_percentage`` reports the percentage of adjacent pairs whose absolute
difference is no larger than the supplied tolerance (at least two samples required).

Data files accept rectangular tab- or comma-separated finite numeric values, including
a final line without a newline. Invalid tokens and insufficient destination capacity
return errors. Writing uses enough decimal precision to round-trip doubles.

Python API Reference
----------------------

brainflow.board\_shim
~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: brainflow.board_shim
   :members:
   :noindex:
   :show-inheritance:
   :member-order: bysource

brainflow.exit\_codes
~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: brainflow.exit_codes
   :members:
   :undoc-members:
   :noindex:
   :show-inheritance:
   :member-order: bysource

brainflow.data\_filter
~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: brainflow.data_filter
   :members:
   :noindex:
   :show-inheritance:
   :member-order: bysource

brainflow.ml\_model
~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: brainflow.ml_model
   :members:
   :noindex:
   :show-inheritance:
   :member-order: bysource

C++ API Reference
-------------------

BoardShim class
~~~~~~~~~~~~~~~~

.. doxygenclass:: BoardShim
   :members:
   :project: BrainFlowCpp

DataFilter class
~~~~~~~~~~~~~~~~~~

.. doxygenclass:: DataFilter
   :members:
   :project: BrainFlowCpp

MLModel class
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: MLModel
   :members:
   :project: BrainFlowCpp

BrainFlow constants
~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../src/utils/inc/brainflow_constants.h
   :language: cpp

Java API Reference
-------------------

Content of Brainflow Package:

.. doxygennamespace:: brainflow
   :project: BrainFlowJava
   :members:
   :content-only:


C# API Reference
-------------------

Content of brainflow namespace:

.. doxygennamespace:: brainflow
   :project: BrainFlowCsharp
   :members:
   :content-only:

R API Reference
-----------------

R binding is a wrapper on top of Python binding. It is implemented using `reticulate <https://rstudio.github.io/reticulate/>`_.

Check R samples to see how to use it.

Full code for R binding:

.. literalinclude:: ../r_package/brainflow/R/package.R
   :language: r

Matlab API Reference
----------------------

Matlab binding calls C/C++ code as any other binding, it's not compatible with Octave.

A few general rules to keep in mind:

- Use char arrays instead strings to work with BrainFlow API, it means :code:`'my_string'` instead :code:`"my_string"`, otherwise you will get calllib error
- Use int32 values intead enums, it means :code:`int32 (BoardIDs.SYNTHETIC_BOARD)` instead :code:`BoardIDs.SYNTHETIC_BOARD`, the same is true for all enums in BrainFlow API

.. mat:automodule:: brainflow
   :members:
   :show-inheritance:


Julia API Reference
---------------------

Julia binding calls C/C++ code as any other binding. Use Julia examples and API reference for other languaes as a starting point.

Since Julia is not Object-Oriented language, there is no DataFilter class. BoardShim class exists but all BoardShim class methods were moved to BrainFlow package and you need to pass BoardShim object to them.

Example:

.. literalinclude:: ../julia_package/brainflow/test/serialization.jl
   :language: julia

Rust API Reference
---------------------

Rust binding calls C/C++ code as any other binding. Use Rust examples and API reference for other languaes as a starting point.

Example:

.. literalinclude:: ../rust_package/brainflow/examples/get_data.rs
   :language: rust

Typescript API Reference
--------------------------

Typescript binding calls C/C++ code as any other binding. Use Typescript examples and API reference for other languaes as a starting point.

Example:

.. literalinclude:: ../nodejs_package/tests/brainflow_get_data.ts
   :language: javascript

Swift
------

Swift binding calls C/C++ code as any other binding. The Swift package exposes BoardShim, DataFilter, MLModel, params, errors, and BrainFlow constants using the same public API groups as Python and Java. In-place signal-processing methods use Swift :code:`inout [Double]` arguments.

Example:

.. literalinclude:: ../swift_package/examples/tests/brainflow_get_data/brainflow_get_data.swift
   :language: swift
