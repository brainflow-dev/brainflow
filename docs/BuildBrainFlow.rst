.. _installation-label:

Installation Instructions
==========================

Precompiled libraries in package managers(Nuget, PYPI, etc)
-------------------------------------------------------------

Core part of BrainFlow is written in C/C++ and distributed as dynamic libraries, for some programming languages we publish packages with precompiled libraries to package managers like Nuget or PYPI.

C/C++ code should be compiled for each CPU architecture and for each OS and we cannot cover all possible cases, as of right now we support:

- x64 libraries for Windows starting from 8.1, for some devices newer version of Windows can be required
- x64 libraries for Linux, they are compiled inside manylinux docker container
- x64/ARM libraries for MacOS, they are universal binaries

If your CPU and OS is not listed above(e.g. Raspberry Pi or Windows with ARM)  you still can use BrainFlow, but you need to compile it by yourself first.

Python
-------

.. compound::

    Please, make sure to use Python 3+. Next, install the latest release from PYPI with the following command in terminal ::

        python -m pip install brainflow

.. compound::

    If you want to install it from source files or build unreleased version from Github, you should compile core module first and run ::

        cd python-package
        python -m pip install -U .

C#
----

**Windows(Visual Studio)**

You are able to install the latest release from `Nuget <https://www.nuget.org/packages/brainflow/>`_ or build it yourself:

- Compile BrainFlow's core module
- Open Visual Studio Solution
- Build it using Visual Studio
- **Make sure that unmanaged(C++) libraries exist in search path** - set PATH env variable or copy them to correct folder

**Unix(Mono)**

- Compile BrainFlow's core module
- Install Mono on your system
- Build it using Mono
- **Make sure that unmanaged(C++) libraries exist in search path** - set LD_LIBRARY_PATH env variable or copy them to correct folder

.. compound::

    Example for Fedora: ::

        # compile c++ code
        tools/build_linux.sh
        # install dependencies, we skip dnf configuration steps 
        sudo dnf install nuget
        sudo dnf install mono-devel
        sudo dnf install mono-complete
        sudo dnf install monodevelop
        # build solution
        xbuild csharp-package/brainflow/brainflow.sln
        # run tests
        export LD_LIBRARY_PATH=/home/andreyparfenov/brainflow/installed_linux/lib/
        mono csharp-package/brainflow/denoising/bin/Debug/test.exe

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

Also, you can use `GitHub Package <https://github.com/brainflow-dev/brainflow/packages/450100>`_ and download BrainFlow using Maven or Gradle.
To use Github packages you need to `change Maven settings <https://help.github.com/en/packages/using-github-packages-with-your-projects-ecosystem/configuring-apache-maven-for-use-with-github-packages>`_. `Example file <https://github.com/brainflow-dev/brainflow/blob/master/java-package/brainflow/settings.xml>`_  here you need to change OWNER and TOKEN by Github username and token with an access to Github Packages.

Matlab
--------

Steps to setup Matlab binding for BrainFlow:

- Compile Core Module, using instructions below. If you don't want to compile C++ code you can download Matlab package with precompiled libs from `Release page <https://github.com/brainflow-dev/brainflow/releases>`_
- Open Matlab IDE and open brainflow/matlab-package/brainflow folder there
- Add folders lib and inc to Matlab path
- If you want to run Matlab scripts from folders different than brainflow/matlab-package/brainflow you need to add it to your Matlab path too
- If you see errors you may need to configure Matlab to use C++ compiler instead C, run this command in Matlab terminal :code:`mex -setup cpp` followed by :code:`MinGW64 Compiler (C++)` or any other C++ compiler. More info can be found `here <https://www.mathworks.com/help/matlab/matlab_external/choose-c-or-c-compilers.html>`_.

Julia
--------

BrainFlow is a registered package in the Julia general registry, so it can be installed via the Pkg manager:

.. compound::

    Example: ::

        import Pkg
        Pkg.add("BrainFlow")
        
When using BrainFlow for the first time in Julia, the BrainFlow artifact containing the compiled BrainFlow libraries will be downloaded from release page automatically.

If you compile BrainFlow from source local libraries will take precedence over the artifact.

Rust
-------

.. compound::

    You can build Rust binding locally using commands below, but you need to compile C/C++ code first ::

        cd rust-package
        cd brainflow
        cargo build --features generate_binding

Docker Image
--------------

There are docker images with precompiled BrainFlow. You can get them from `DockerHub <https://hub.docker.com/r/brainflow/brainflow>`_.

All bindings except Matlab are preinstalled there.

Also, there are other packages for BCI research and development:

- mne
- pyriemann
- scipy
- matplotlib
- jupyter
- pandas
- etc

If your devices uses TCP/IP to send data, you need to run docker container with :code:`--network host`. For serial port connection you need to pass serial port to docker using :code:`--device %your port here%`

.. compound::

    Example:  ::

        # pull container from DockerHub
        docker pull brainflow/brainflow:latest
        # run docker container with serial port /dev/ttyUSB0
        docker run -it --device /dev/ttyUSB0 brainflow/brainflow:latest /bin/bash
        # run docker container for boards which use networking
        docker run -it --network host brainflow/brainflow:latest /bin/bash

Compilation of Core Module and C++ Binding
-------------------------------------------

Windows
~~~~~~~~

- Install CMake>=3.16 you can install it from PYPI via pip or from `CMake website <https://cmake.org/>`_
- Install Visual Studio 2019(preferred) or Visual Studio 2017. Other versions may work but not tested
- In VS installer make sure you selected "Visual C++ ATL support"
- Build it as a standard CMake project, you don't need to set any options

.. compound::

    If you are not familiar with CMake you can use `build.py <https://github.com/brainflow-dev/brainflow/blob/master/tools/build.py>`_ : ::

        # install python3 and run
        python -m pip install cmake
        cd tools
        python build.py
        # to get info about args and configure your build you can run
        python build.py --help


Linux
~~~~~~

- Install CMake>=3.16 you can install it from PYPI via pip, via package managers for your OS(apt, dnf, etc) or from `CMake website <https://cmake.org/>`_
- If you are going to distribute compiled Linux libraries you HAVE to build it inside manylinux Docker container
- Build it as a standard CMake project, you don't need to set any options
- You can use any compiler but for Linux we test only GCC

.. compound::

    If you are not familiar with CMake you can use `build.py <https://github.com/brainflow-dev/brainflow/blob/master/tools/build.py>`_ : ::

        python3 -m pip install cmake
        cd tools
        python3 build.py
        # to get info about args and configure your build you can run
        python3 build.py --help

MacOS
~~~~~~~

- Install CMake>=3.16 you can install it from PYPI via pip, using :code:`brew` or from `CMake website <https://cmake.org/>`_
- Build it as a standard CMake project, you don't need to set any options
- You can use any compiler but for MacOS we test only Clang

.. compound::

    If you are not familiar with CMake you can use `build.py <https://github.com/brainflow-dev/brainflow/blob/master/tools/build.py>`_ : ::

        python3 -m pip install cmake
        cd tools
        python3 build.py
        # to get info about args and configure your build you can run
        python3 build.py --help


Android
---------

To check supported boards for Android visit :ref:`supported-boards-label`

Installation instructions
~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Create Java project in Android Studio, Kotlin is not supported
- Download *jniLibs.zip* from `Release page <https://github.com/brainflow-dev/brainflow/releases>`_
- Unpack *jniLibs.zip* and copy it's content to *project/app/src/main/jniLibs*
- Download *brainflow-jar-with-dependencies.jar* from `Release page <https://github.com/brainflow-dev/brainflow/releases>`_  or from `Github package <https://github.com/brainflow-dev/brainflow/packages/290893>`_
- Copy *brainflow-jar-with-dependencies.jar* to *project/app/libs folder*

Now you can use BrainFlow SDK in your Android application!

Note: Android Studio inline compiler may show red errors but it should be compiled fine with Gradle. To fix inline compiler you can use *File > Sync Project with Gradle Files* or click at *File > Invalidate Cache/Restart > Invalidate and Restart*

.. compound::
    
    For some API calls you need to provide additional permissions via manifest file of your application ::

        <uses-permission android:name="android.permission.INTERNET"></uses-permission>
        <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"></uses-permission>
        <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"></uses-permission>
        <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"></uses-permission>


Compilation using Android NDK
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**For BrainFlow developers**


To test your changes in BrainFlow on Android you need to build it using Android NDK manually.

Compilation instructions:

- `Download Android NDK <https://developer.android.com/ndk/downloads>`_
- `Download Ninja <https://github.com/ninja-build/ninja/releases>`_ or get one from the *tools* folder, make sure that *ninja.exe*  is in search path
- You can also try *MinGW Makefiles* instead *Ninja*, but it's not tested and may not work
- Build C++ code using cmake and *Ninja* for **all ABIs**
- Compiled libraries will be in *tools/jniLibs* folder

.. compound::
    
    Command line examples: ::

        # to prepare project(choose ABIs which you need)
        # for arm64-v8a
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=E:\android-ndk-r21d-windows-x86_64\android-ndk-r21d\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-19 -DANDROID_ABI=arm64-v8a ..
        # for armeabi-v7a
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=E:\android-ndk-r21d-windows-x86_64\android-ndk-r21d\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-19 -DANDROID_ABI=armeabi-v7a ..
        # for x86_64
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=E:\android-ndk-r21d-windows-x86_64\android-ndk-r21d\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-19 -DANDROID_ABI=x86_64 ..
        # for x86
        cmake -G Ninja -DCMAKE_TOOLCHAIN_FILE=E:\android-ndk-r21d-windows-x86_64\android-ndk-r21d\build\cmake\android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-19 -DANDROID_ABI=x86 ..

        # to build(should be run for each ABI from previous step**
        cmake --build . --target install --config Release -j 2 --parallel 2
