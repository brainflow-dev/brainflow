User API
==========

BrainFlow User API has no classes for all supported boardm instead there are only two classes:
- BoardShim to read data from a board
- DataFilter to perform signal processing

To select a board you need to pass BrainFlow's board id to BoardShim's constructor as well as required parameters for this particular board.
This level of abstraction allows you to change board almost without any changes in code.

Since API for all bindings is almost the same we add description for parameters, return values and methods only for Python API.
For other languages we add docs without any description and hints.

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

Cpp API Reference
-------------------

Here we describe only high level API, to get information about structures, classes and enums from low level API check Brainflow Core API

BoardShim class
~~~~~~~~~~~~~~~~

This class communicates with a real board and read/send data from/to it

.. doxygenclass:: BoardShim
   :members:
   :project: BrainFlowCpp
   :undoc-members:

DataFilter class
~~~~~~~~~~~~~~~~~~

Use this for signal processing, all methods change input data in-place.

.. doxygenclass:: DataFilter
   :members:
   :project: BrainFlowCpp
   :undoc-members:

BrainFlowException class
~~~~~~~~~~~~~~~~~~~~~~~~~

Exception which is raised if wrong exit code was returned from low level API

.. doxygenclass:: BrainFlowException
   :members:
   :project: BrainFlowCpp
   :undoc-members:


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

To simplify 2D array manipulation we use `Accord Library <http://accord-framework.net/>`_. You need to install it first from Nuget.

Content of brainflow namespace:

.. doxygennamespace:: brainflow
   :project: BrainFlowCsharp
   :undoc-members:
   :members:
   :content-only:

R API Reference
-----------------

R binding is a wrapper on top of Python binding which is implemented using `reticulate <https://rstudio.github.io/reticulate/>`_.
There are a few methods which allows you to create python objects and call their methods. 

But reticulate translates numpy arrays to R arrays in a tricky way, and it prevents us to implement signal processing in R, so you for R you have to perform signal processing by yourself.

Check R sample to see how to use it.

Full code for R binding:

.. literalinclude:: ../r-package/brainflow/R/board_shim.R
   :language: r