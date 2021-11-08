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
   :undoc-members:

DataFilter class
~~~~~~~~~~~~~~~~~~

.. doxygenclass:: DataFilter
   :members:
   :project: BrainFlowCpp
   :undoc-members:

MLModel class
~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenclass:: MLModel
   :members:
   :project: BrainFlowCpp
   :undoc-members:

BrainFlow constants
~~~~~~~~~~~~~~~~~~~~~

.. literalinclude:: ../src/utils/inc/brainflow_constants.h
   :language: cpp

Java API Reference
-------------------

Content of Brainflow Package:

.. doxygennamespace:: brainflow
   :project: BrainFlowJava
   :undoc-members:
   :members:
   :content-only:


C# API Reference
-------------------

Content of brainflow namespace:

.. doxygennamespace:: brainflow
   :project: BrainFlowCsharp
   :undoc-members:
   :members:
   :content-only:

R API Reference
-----------------

R binding is a wrapper on top of Python binding. It is implemented using `reticulate <https://rstudio.github.io/reticulate/>`_.

Check R samples to see how to use it.

Full code for R binding:

.. literalinclude:: ../r-package/brainflow/R/package.R
   :language: r

Matlab API Reference
----------------------

Matlab binding calls C/C++ code as any other binding, it's not compatible with Octave.

A few general rules to keep in mind:

- Use char arrays instead strings to work with BrainFlow API, it means :code:`'my_string'` instead :code:`"my_string"`, otherwise you will get calllib error
- Use int32 values intead enums, it means :code:`int32 (BoardIDs.SYNTHETIC_BOARD)` instead :code:`BoardIDs.SYNTHETIC_BOARD`, the same is true for all enums in BrainFlow API

.. mat:automodule:: brainflow
   :members:
   :undoc-members:
   :show-inheritance:


Julia API Reference
---------------------

Julia binding calls C/C++ code as any other binding. Use Julia examples and API reference for other languaes as a starting point.

Since Julia is not Object-Oriented language, there is no DataFilter class. BoardShim class exists but all BoardShim class methods were moved to BrainFlow package and you need to pass BoardShim object to them.

Example:

.. literalinclude:: ../julia-package/brainflow/test/serialization.jl
   :language: julia

Rust API Reference
---------------------

Rust binding calls C/C++ code as any other binding. Use Rust examples and API reference for other languaes as a starting point.

Example:

.. literalinclude:: ../rust-package/brainflow/examples/get_data.rs
   :language: rust
