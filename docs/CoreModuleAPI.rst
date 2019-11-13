BrainFlow Core API
===================

Core module of BrainFlow API is represented by two dynamic libraries:

* BoardController for data acquisition
* DataHandler for signal processing

These libraries are independent and you can use only one of them if you want to.

All possible exit codes and supported boards
----------------------------------------------

.. literalinclude:: ../src/board_controller/inc/brainflow_constants.h
   :language: cpp

BoardController Interface
---------------------------

This library reads data from a board and provides an API to request information about this board and data format. In User API these methods are available via BoardShim class.

Data acquisition methods are described below:

.. literalinclude:: ../src/board_controller/inc/board_controller.h
   :language: cpp

Board Description methods are described below:

.. literalinclude:: ../src/board_controller/inc/board_info_getter.h
   :language: cpp

DataHandler Interface
----------------------

This library performs signal processing. In User API these methods are available via DataFilter class.

.. literalinclude:: ../src/data_handler/inc/data_handler.h
   :language: cpp
