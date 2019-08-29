Build Instructions
==================

Common requirements
----------------------

- Windows >= 8.1/Linux/MacOS
- On Windows we support x86 and x64 libraries for Python and C# for other languages - x64 only, for MacOS and Linux - only x64 for all languages
- For Ganglion on Windows if you dont compile brainflow by yorself you may need to install `redist_x64 <https://aka.ms/vs/16/release/vc_redist.x64.exe>`_ or `redist_x86 <https://aka.ms/vs/16/release/vc_redist.x86.exe>`_  (but more likely you have it preinstalled) to use built-in Bluetooth API also you need Windows >= 10
- For Ganglion on Linux/MacOS you need to use dongle, for Windows dongle is not required if you have Bluetooth on your laptop

Compilation
-------------

Windows
~~~~~~~

- Install CMAKE>=3.10 you can install it from PYPI via pip
- Install Visual Studio 2017
- run ./tools/build_win64.cmd or ./tools/build_win32.cmd from project directory, if you wanna use another version of Visual Studio you will need to change generator in cmake command line

Linux
~~~~~~
- Install CMAKE>=3.10
- If you wanna distribute compiled Linux linraries you HAVE to build it inside manylinux Docker container
- run .\\tools\\build_linux.sh

Python
-------

.. compound::

    After compilation of BrainFlow's core module you need to run ::

       	cd python-package
      	python -m pip install -e .

.. compound::

    Also you are able to install latest release from PYPI ::

      	python -m pip install brainflow

R
-----

R binding is based on `reticulate <https://rstudio.github.io/reticulate/>`_ package and calls python code, so you need to install Python binding first, if you install it in virtualenv make sure that reticulate use correct virtualenv, after that you will be able to build R package from command line or using R Studio, install it and run samples

Java
-----

.. compound::

    After compilation of BrainFlow's core module you need to install maven and run ::

       	cd java-package
      	mvn package

Also you can download jar files with all dependencies inside directly from `release page <https://github.com/OpenBCI/brainflow/releases>`_

C#
----

For C# only Windows is currently supported

- Compile BrainFlow's core module
- open Visual Studio Solution
- install required nuget packages
- build it using Visual Studio
- make sure that unmanaged libraries exist in search Path

Unity Integration
~~~~~~~~~~~~~~~~~~

- build C# package
- copy managed DLLs to the Assets folder of your Unity project
- copy unmanaged DLLS to project folder to use it with Unity player
- after building Unity project copy unmanaged DLLs to exe folder

Matlab
-------

- Compile BrainFlow's core module
- run samples
