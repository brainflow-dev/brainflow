Installation Instructions
==========================

Python
-------

.. compound::

    You are able to install latest release from PYPI ::

        python -m pip install brainflow

.. compound::

    If you want to install it from source files or build unreleased version from github you should compile core module first and run ::

       	cd python-package
      	python -m pip install -e .

C#
----

For C# only Windows is currently supported.

You are able to install latest release from `Nuget <https://www.nuget.org/packages/brainflow/>`_ or build it yourself:

- Compile BrainFlow's core module
- open Visual Studio Solution
- install required nuget packages
- build it using Visual Studio
- make sure that unmanaged libraries exist in search path

Unity Integration
~~~~~~~~~~~~~~~~~~

- build C# package
- copy managed DLLs to the Assets folder of your Unity project
- copy unmanaged DLLS to project folder to use it with Unity player
- after building Unity project copy unmanaged DLLs to exe folder

R
-----

R binding is based on `reticulate <https://rstudio.github.io/reticulate/>`_ package and calls python code, so you need to install Python binding first, make sure that reticulate uses correct virtual environment, after that you will be able to build R package from command line or using R Studio, install it and run samples.

Java
-----

You are able to download jar files directly from `release page <https://github.com/Andrey1994/brainflow/releases>`_

.. compound::

    If you want to install it from source files or build unreleased version from github you should compile core module first and run ::

       	cd java-package
      	mvn package


Compilation of Core module and C++ binding
-------------------------------------------

Windows
~~~~~~~~

- Install Cmake>=3.10 you can install it from PYPI via pip
- Install Visual Studio 2017, you can use another version but you will need to change cmake generator in batch files or run cmake commands manually. Also in CI we test only VS2017
- Build it as a cmake project manually or use batch files from tools directory

.. compound::

    Compilation using batch files ::

        python -m pip install cmake==3.13.0
        .\tools\build_win32.cmd
        .\tools\build_win64.cmd

Linux
~~~~~~

- Install Cmake>=3.10 you can install it from PYPI via pip
- If you wanna distribute compiled Linux linraries you HAVE to build it inside manylinux Docker container
- Build it as a cmake project manually or use bash file from tools directory
- You can use any compiler but for Linux we test only GCC, also we test only 64bit libraries for Linux

.. compound::

    Compilation using bash file ::

        python -m pip install cmake==3.13.0
        ./tools/build_linux.sh

MacOS
~~~~~~~

- Install Cmake>=3.10 you can install it from PYPI via pip
- Build it as a cmake project manually or use bash file from tools directory
- You can use any compiler but for MacOS we test only Clang

.. compound::

    Compilation using bash file ::

        python -m pip install cmake==3.13.0
        ./tools/build_mac.sh

