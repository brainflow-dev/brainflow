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

**Windows(Visual Studio)**

You are able to install the latest release from `Nuget <https://www.nuget.org/packages/brainflow/>`_ or build it yourself:

- Compile BrainFlow's core module
- open Visual Studio Solution
- install required nuget packages
- build it using Visual Studio
- **make sure that unmanaged(C++) libraries exist in search path** - set PATH env variable or copy them to correct folder

**Unix(Mono)**

- Compile BrainFlow's core module
- install nuget and Mono on your system
- install required nuget packages
- build it using Mono
- **make sure that unmanaged(C++) libraries exist in search path** - set LD_LIBRARY_PATH env variable or copy them to correct folder

.. compound::

    Example for Fedora: ::

        # compile c++ code
        tools/build_linux.sh
        # install dependencies, we skip dnf configuration steps 
        sudo dnf install nuget
        sudo dnf install mono-devel
        sudo dnf install mono-complete
        sudo dnf install monodevelop
        # install nuget packages
        nuget restore csharp-package/brainflow/brainflow.sln
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

- Install CMake>=3.13 you can install it from PYPI via pip
- Install Visual Studio 2017, you can use another version but you will need to change CMake generator in batch files or run CMake commands manually. Also in CI we test only VS2017
- In VS installer make sure you selected "Visual C++ ATL support"
- Build it as a CMake project manually or use cmd files from tools directory

.. compound::

    Compilation using cmd files: ::

        python -m pip install cmake
        # need to run these files from project dir
        .\tools\build_win32.cmd
        .\tools\build_win64.cmd

Linux
~~~~~~

- Install CMake>=3.13 you can install it from PYPI via pip
- If you wanna distribute compiled Linux libraries you HAVE to build it inside manylinux Docker container
- Build it as a CMake project manually or use bash file from tools directory
- You can use any compiler but for Linux we test only GCC, also we test only 64bit libraries for Linux

.. compound::

    Compilation using bash file: ::

        python -m pip install cmake
        # you may need to change line endings using dos2unix or text editor for file below
        # need to run this file from project dir
        bash ./tools/build_linux.sh

MacOS
~~~~~~~

- Install CMake>=3.13 you can install it from PYPI via pip
- Build it as a CMake project manually or use bash file from tools directory
- You can use any compiler but for MacOS we test only Clang

.. compound::

    Compilation using bash file: ::

        python -m pip install cmake
        # you may need to change line endings using dos2unix or text editor for file below
        # need to run this file from project dir
        bash ./tools/build_mac.sh



Compilation with OpenMP
~~~~~~~~~~~~~~~~~~~~~~~~~~

Some data processing and machine learning algorithms work much faster if you run them in multiple threads. To parallel computations we use OpenMP library.

**Precompiled libraries which you download from PYPI/Nuget/Maven/etc built without OpenMP support and work in single thread.**

If you want to increase performance of signal processing algorithms you can compile BrainFlow from the source and turn on *USE_OPENMP* option.

To build BrainFlow with OpenMP support first of all you need to install OpenMP.

- On Windows all you need is Visual C++ Redist package which is installed automatically with Visual Studio
- On Linux you may need to install libgomp if it's not currently installed
- On MacOS you need to run :code:`brew install libomp`

After that you need to compile BrainFlow with OpenMP support, steps are exactly the same as above, but you need to run bash or cmd scripts whith _omp postfix.

.. compound::

    Example: ::

        # for Linux
        bash ./tools/build_linux_omp.sh
        # for MacOS
        bash ./tools/build_mac_omp.sh
        # for Windows
        .\tools\build_win64_omp.cmd

If you use CMake directly to build BrainFlow you need to add :code:`-DUSE_OPENMP=ON` to CMake config command line.


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

        # to build(should be run for each ABI from previous step)
        cmake --build . --target install --config Release -j 2 --parallel 2
