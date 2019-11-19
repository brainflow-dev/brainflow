
Code Samples
============

Python sample
---------------

.. literalinclude:: ../python-package/examples/brainflow_get_data.py
   :language: py

R sample
---------

For R BrainFlow's signal processing API is unavailable, you will need to implement filters by yourself or use packages from CRAN

.. literalinclude:: ../r-package/examples/brainflow_get_data.R
	:language: r

Java sample
-------------

.. literalinclude:: ../java-package/brainflow/src/test/java/brainflow_test/BrainFlowTest.java
   :language: java


C# sample
-----------

.. literalinclude:: ../csharp-package/brainflow/test/get_board_data.cs
   :language: c#


C++ sample
-----------

**Make sure that compiled dynamic libraries exist in search path before running an executable, do one of:**

- for Linux and MacOS add them to LD_LIBRARY_PATH env variable
- for Windows add them to PATH env variable
- or just copypaste them to the folder where your executable is located


.. literalinclude:: ../tests/cpp/CMakeLists.txt
   :language: none

.. literalinclude:: ../tests/cpp/src/brainflow_get_data.cpp
   :language: cpp
