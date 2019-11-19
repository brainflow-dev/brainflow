User API
==========

BrainFlow User API has only two classes:

- BoardShim to read data from a board, it calls methods from underlying BoardController library
- DataFilter to perform signal processing, it calls methods from underlying DataHandler library

These classes are independent, so if you want, you can use BrainFlow API only for data streaming and perform signal processing by yourself and vice versa.

BrainFlow API is board agnostic, so to select a specific board you need to pass BrainFlow's board id to BoardShim's constructor and an instance of BrainFlowInputParams structure which should hold information for your specific board, check Supported Boards section for details.
This abstraction allows you to switch boards without any changes in code.

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

BrainFlowException class
~~~~~~~~~~~~~~~~~~~~~~~~~

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

To simplify 2D array manipulation we use `Accord Library <http://accord-framework.net/>`_. 

Content of brainflow namespace:

.. doxygennamespace:: brainflow
   :project: BrainFlowCsharp
   :undoc-members:
   :members:
   :content-only:

R API Reference
-----------------

R binding is a wrapper on top of Python binding, it is implemented using `reticulate <https://rstudio.github.io/reticulate/>`_.
There are a few methods which allow you to create python objects and call their methods. 

Reticulate translates numpy arrays to R arrays in a tricky way, and it prevents us to implement signal processing in R, so for R you have to perform signal processing by yourself.

Check R sample to see how to use it.

Full code for R binding:

.. literalinclude:: ../r-package/brainflow/R/board_shim.R
   :language: r
