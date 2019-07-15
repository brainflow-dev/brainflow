User API
=========

There is no separated classes for each BCI board, instead there is only one BoardShim class, you need to pass BrainFlow's board_id to BoardShim constructor to choose a board.
Using this design you are able to switch boards via single integer value and that's all.
To request information like sampling rate for speciffic board we recommend to use BoardInfoGetter class.

All bindings have pretty similar API, only important difference - java and matlab bindings currently have no signal processing methods.
All bindings throw BrainFlowException with exit code and error message if smth goes wrong, to check all possible exit codes and their description visit github page

Python
--------

.. literalinclude:: ../python-package/examples/brainflow_get_data.py
   :language: py

R
-----

.. literalinclude:: ../r-package/examples/brainflow_get_data.R
   :language: r

Java
-----

.. literalinclude:: ../java-package/brainflow/src/test/java/BrainFlowTest.java
   :language: java


C#
-----

.. literalinclude:: ../csharp-package/brainflow/test/get_board_data.cs
   :language: c#


Unity Integration
~~~~~~~~~~~~~~~~~~

- compile core module
- compile c# binding
- copy managed DLLs to Assets folder
- copy unmanaged DLLS to project folder to use it with Unity player
- after building Unity project copy unmanaged DLLs to exe folder

Matlab
-------

.. literalinclude:: ../matlab-package/BrainflowGetData.m
   :language: matlab


C++
-----

Instructions:

.. literalinclude:: ../tests/cpp/CMakeLists.txt
   :language: none

.. literalinclude:: ../tests/cpp/src/brainflow_get_data.cpp
   :language: cpp