Build Instructions
==================

You are able to download compiled libraries from github release page or build BrainFlow by yourself

Manual Compilation (Core Module + CPP package)
-----------------------------------------------

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

    After compilation of BrainFlow's core module or manual copypasting of compiled libraries you need to run ::

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

    After compilation of BrainFlow's core module or manual copypasting of compiled libraries you need to install maven and run ::

       	cd java-package
      	mvn package

C#
----

- Compile BrainFlow's core module or copypaste libs
- open Visual Studio Solution
- install required nuget packages
- build it using Visual Studio
- make sure that unmanaged libraries exist in search Path

Matlab
-------

- Compile BrainFlow's core module or copypaste libs
- run samples
