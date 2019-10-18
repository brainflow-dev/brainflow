User API
==========

BrainFlow User API has no classes for all supported boardm instead there are only two classes:
- BoardShim to read data from a board
- DataFilter to perform signal processing

To select a board you need to pass BrainFlow's board id to BoardShim's constructor as well as required parameters for this particular board.
This level of abstraction allows you to change board almost without any changes in code.

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
~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: brainflow.data_filter
   :members:
   :undoc-members:
   :noindex:
   :show-inheritance:
   :member-order: bysource
