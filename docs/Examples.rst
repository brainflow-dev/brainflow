.. _code-samples-label:

Code Samples
=============

Make sure that you've installed BrainFlow package before running the code samples below.

See :ref:`installation-label` for details.

Python
--------

**To run some signal processing samples, you may need to install:**

- matplotlib
- pandas
- mne
- pyqtgraph

BrainFlow doesn't use these packages and doesn't install them, but the packages will be used in demos below.

Python Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/brainflow_get_data.py
   :language: py

Python Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/markers.py
   :language: py

Python Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/serialization.py
   :language: py

Python Downsample Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/downsampling.py
   :language: py

Python Transforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/transforms.py
   :language: py

Python Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/signal_filtering.py
   :language: py

Python Denoising
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/denoising.py
   :language: py

Python MNE Integration
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/brainflow_to_mne.py
   :language: py

Python Band Power
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/band_power.py
   :language: py

Python EEG Metrics
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/eeg_metrics.py
   :language: py

Python ICA
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../python_package/examples/tests/ica.py
   :language: py

Python Real Time Plot
~~~~~~~~~~~~~~~~~~~~~~~~

Extra requirements for this code sample:

.. literalinclude:: ../python_package/examples/plot_real_time/requirements.txt
   :language: none

.. literalinclude:: ../python_package/examples/plot_real_time/plot_real_time_min.py
   :language: py

Java
------

Java Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/BrainFlowGetData.java
   :language: java

Java Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/Markers.java
   :language: java

Java Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/Serialization.java
   :language: java

Java Downsample Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/Downsampling.java
   :language: java

Java Transforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/Transforms.java
   :language: java

Java Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/SignalFiltering.java
   :language: java

Java Denoising
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/Denoising.java
   :language: java

Java Band Power
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/BandPower.java
   :language: java

Java EEG Metrics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/EEGMetrics.java
   :language: java

Java ICA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../java_package/brainflow/src/main/java/brainflow/examples/ICA.java
   :language: java

C#
-----

C# Read Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/brainflow_get_data/brainflow_get_data.cs
   :language: c#

C# Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/markers/markers.cs
   :language: c#

C# Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/serialization/serialization.cs
   :language: c#

C# Downsample Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/downsampling/downsampling.cs
   :language: c#

C# Transforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/transforms/transforms.cs
   :language: c#

C# Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/signal_filtering/signal_filtering.cs
   :language: c#

C# Denoising
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/denoising/denoising.cs
   :language: c#

C# Band Power
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/band_power/band_power.cs
   :language: c#

C# EEG Metrics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/eeg_metrics/eeg_metrics.cs
   :language: c#

C# ICA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../csharp_package/brainflow/examples/ica/ica.cs
   :language: c#

C++
-----

.. compound::

   To compile examples below for Linux or MacOS run: ::

      cd cpp_package/examples/get_data
      mkdir build
      cd build
      cmake -DCMAKE_PREFIX_PATH=TYPE_FULL_PATH_TO_BRAINFLOW_INSTALLED_FOLDER ..
      # e.g. cmake -DCMAKE_PREFIX_PATH=/home/andrey/brainflow/installed ..
      make

For Windows it's almost the same.

**Make sure that compiled dynamic libraries exist in search path before running an executable by doing one of the following:**

- for Linux and MacOS add them to LD_LIBRARY_PATH env variable
- for Windows add them to PATH env variable
- or just copypaste them to the folder where your executable is located

CMake File Example
~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/get_data/CMakeLists.txt
   :language: none

C++ Read Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/get_data/src/brainflow_get_data.cpp
   :language: cpp

C++ Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/get_data/src/markers.cpp
   :language: cpp

C++ Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/signal_processing/src/serialization.cpp
   :language: cpp

C++ Downsample Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/signal_processing/src/downsampling.cpp
   :language: cpp

C++ Transforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/signal_processing/src/transforms.cpp
   :language: cpp

C++ Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/signal_processing/src/signal_filtering.cpp
   :language: cpp

C++ Denoising
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/signal_processing/src/denoising.cpp
   :language: cpp

C++ Band Power
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/signal_processing/src/band_power.cpp
   :language: cpp

C++ EEG Metrics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/ml/src/eeg_metrics.cpp
   :language: cpp

C++ ICA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../cpp_package/examples/signal_processing/src/ica.cpp
   :language: cpp


R
----

R Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/brainflow_get_data.R
   :language: r

R Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/markers.R
   :language: r

R Read Write File
~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/serialization.R
   :language: r

R Transforms
~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/transforms.R
   :language: r

R Signal Filtering
~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/signal_filtering.R
   :language: r

R Denoising
~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/denoising.R
   :language: r

R Band Power
~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/band_power.R
   :language: r

R EEG Metrics
~~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/eeg_metrics.R
   :language: r

R ICA
~~~~~~~~~~~~~~

.. literalinclude:: ../r_package/examples/ICA.R
   :language: r

Matlab
--------

Matlab Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/BrainFlowGetData.m
   :language: matlab

Matlab Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/Markers.m
   :language: matlab

Matlab Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/Serialization.m
   :language: matlab

Matlab Transforms
~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/Transforms.m
   :language: matlab

Matlab Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/SignalFiltering.m
   :language: matlab

Matlab Denoising
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/Denoising.m
   :language: matlab

Matlab Band Power
~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/BandPower.m
   :language: matlab

Matlab EEG Metrics
~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/EEGMetrics.m
   :language: matlab

Matlab ICA
~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../matlab_package/brainflow/examples/ICA.m
   :language: matlab

Julia
--------

Julia Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/brainflow_get_data.jl
   :language: julia

Julia Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/markers.jl
   :language: julia

Julia Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/serialization.jl
   :language: julia

Julia Transforms
~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/transforms.jl
   :language: julia

Julia Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/signal_filtering.jl
   :language: julia

Julia Denoising
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/denoising.jl
   :language: julia

Julia Band Power
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/band_power.jl
   :language: julia

Julia EEG Metrics
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/eeg_metrics.jl
   :language: julia

Julia ICA
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../julia_package/brainflow/test/ica.jl
   :language: julia

Rust
--------

Rust Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/get_data.rs
   :language: rust

Rust Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/markers.rs
   :language: rust

Rust Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/read_write_file.rs
   :language: rust

Rust Transforms
~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/transforms.rs
   :language: rust

Rust Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/get_data.rs
   :language: rust

Rust Denoising
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/denoising.rs
   :language: rust

Rust Band Power
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/band_power.rs
   :language: rust

Rust EEG Metrics
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/eeg_metrics.rs
   :language: rust

Rust ICA
~~~~~~~~~~~~~~~~~

.. literalinclude:: ../rust_package/brainflow/examples/ica.rs
   :language: rust

Typescript
------------

Typescript Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/brainflow_get_data.ts
   :language: javascript

Typescript Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/markers.ts
   :language: javascript

Typescript Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/serialization.ts
   :language: javascript

Typescript Downsample Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/downsampling.ts
   :language: javascript

Typescript Transforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/transforms.ts
   :language: javascript

Typescript Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/signal_filtering.ts
   :language: javascript

Typescript Denoising
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/denoising.ts
   :language: javascript

Typescript Band Power
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/bandpower.ts
   :language: javascript

Typescript EEG Metrics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/eeg_metrics.ts
   :language: javascript

Typescript ICA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../nodejs_package/tests/ica.ts
   :language: javascript

Swift
------------

Swift Get Data from a Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/brainflow_get_data/brainflow_get_data.swift
   :language: swift

Swift Markers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/markers/markers.swift
   :language: swift

Swift Read Write File
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/read_write_file/read_write_file.swift
   :language: swift

Swift Downsample Data
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/downsampling/downsampling.swift
   :language: swift

Swift Transforms
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/transforms/transforms.swift
   :language: swift

Swift Signal Filtering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/signal_filtering/signal_filtering.swift
   :language: swift

Swift Denoising
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/denoising/denoising.swift
   :language: swift

Swift Band Power
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/band_power/band_power.swift
   :language: swift

Swift EEG Metrics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/eeg_metrics/eeg_metrics.swift
   :language: swift

Swift ICA
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../swift_package/examples/tests/ica/ica.swift
   :language: swift

Notebooks
------------
.. toctree::

   ./notebooks/brainflow_mne
   ./notebooks/denoising
   ./notebooks/band_power
