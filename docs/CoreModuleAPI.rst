BrainFlow Core API
===================

Core module of BrainFlow includes two dynamic libraries, one of them reads data from a board and provides methods to query information about data format while another dynamic library performs signal processing. All bindings just call methods from these dynamic libraries, it simplifies adding new boards and maintenance.

All possible exit codes
-------------------------

.. literalinclude:: ../src/board_controller/inc/brainflow_constants.h
   :language: cpp

Board Controller Interface
---------------------------

This library reads data from a board and provides API to request information about this board and data format. In User API these methods are available via BoardShim class

Data acquisition methods are described below

.. literalinclude:: ../src/board_controller/inc/board_controller.h
   :language: cpp

Board Description methods are described below

.. literalinclude:: ../src/board_controller/inc/board_info_getter.h
   :language: cpp

Data Handler Interface
----------------------

This library performs basic signal processing. In User API these methods are available via DataFilter class

.. literalinclude:: ../src/data_handler/inc/data_handler.h
   :language: cpp
