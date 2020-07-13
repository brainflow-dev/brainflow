.. _installation-label:

Installation Instructions
==========================

Python
-------

.. compound::

    Please, make sure to use Python 3+. Next, install the latest release from PYPI with the following command in terminal ::

        python -m pip install brainflow

.. compound::

    If you want to install it from source files or build unreleased version from Github, you should compile core module first and run ::

        cd python-package
        python -m pip install -e .

C#
----

For C#, only Windows is currently supported.

You are able to install the latest release from `Nuget <https://www.nuget.org/packages/brainflow/>`_ or build it yourself:

- Compile BrainFlow's core module
- open Visual Studio Solution
- install required nuget packages
- build it using Visual Studio
- **make sure that unmanaged(C++) libraries exist in search path** - set PATH env variable or copy them to correct folder

R
-----

R binding is based on `reticulate <https://rstudio.github.io/reticulate/>`_ package and calls Python code, so you need to install Python binding first, make sure that reticulate uses correct virtual environment, after that you will be able to build R package from command line or using R Studio, install it and run samples.

Java
-----

You are able to download jar files directly from `release page <https://github.com/brainflow-dev/brainflow/releases>`_

.. compound::

    If you want to install it from source files or build unreleased version from github you should compile core module first and run ::

        cd java-package
        cd brainflow
        mvn package

Also, you can use `GitHub Package <https://github.com/brainflow-dev/brainflow/packages/290893>`_ and download BrainFlow using Maven or Gradle.
To use Github packages you need to `change Maven settings <https://help.github.com/en/packages/using-github-packages-with-your-projects-ecosystem/configuring-apache-maven-for-use-with-github-packages>`_. `Example file <https://github.com/brainflow-dev/brainflow/blob/master/java-package/brainflow/settings.xml>`_  here you need to change OWNER and TOKEN by Github username and token with an access to Github Packages.

Matlab
--------

Steps to setup Matlab binding for BrainFlow:

- Compile Core Module, using instructions below
- Open Matlab IDE and open brainflow/matlab-package/brainflow folder there
- Add folders lib and inc to Matlab path
- If you want to run Matlab scripts from folders different than brainflow/matlab-package/brainflow you need to add it to your Matlab path too


Julia
--------

Steps to setup Julia binding for BrainFlow:

- Compile Core Module, using instructions below
- Set PATH(on Windows) or LD_LIBRARY_PATH(on Unix) env variables to ensure that compiled libraries are in search path
- Install BrainFlow package locally

.. compound::

    Example ::

        # compile core module first
        # set env variable
        export LD_LIBRARY_PATH=/home/andreyparfenov/brainflow/installed_linux/lib/:$LD_LIBRARY_PATH
        cd julia-package/brainflow
        julia
        # type ']' to switch to pkg terminal
        activate . # activate BrainFlow's env
        


Compilation of Core Module and C++ Binding
-------------------------------------------

Windows
~~~~~~~~

- Install Cmake>=3.13 you can install it from PYPI via pip
- Install Visual Studio 2017, you can use another version but you will need to change cmake generator in batch files or run cmake commands manually. Also in CI we test only VS2017
- Build it as a cmake project manually or use cmd files from tools directory

.. compound::

    Compilation using cmd files ::

        python -m pip install cmake==3.13.3
        # need to run these files from project dir
        .\tools\build_win32.cmd
        .\tools\build_win64.cmd

Linux
~~~~~~

- Install Cmake>=3.13 you can install it from PYPI via pip
- If you wanna distribute compiled Linux libraries you HAVE to build it inside manylinux Docker container
- Build it as a cmake project manually or use bash file from tools directory
- You can use any compiler but for Linux we test only GCC, also we test only 64bit libraries for Linux

.. compound::

    Compilation using bash file ::

        python -m pip install cmake==3.13.3
        # you may need to change line endings using dos2unix or text editor for file below
        # need to run this file from project dir
        bash ./tools/build_linux.sh

MacOS
~~~~~~~

- Install Cmake>=3.13 you can install it from PYPI via pip
- Build it as a cmake project manually or use bash file from tools directory
- You can use any compiler but for MacOS we test only Clang

.. compound::

    Compilation using bash file ::

        python -m pip install cmake==3.13.3
        # you may need to change line endings using dos2unix or text editor for file below
        # need to run this file from project dir
        bash ./tools/build_mac.sh

