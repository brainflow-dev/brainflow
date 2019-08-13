# Brainflow

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG data from OpenBCI boards.

Core module of this library is implemented in C\C++ and available for all bindings as a dynamic library. This approach allows to reduce code duplication and simplify maintenance, also it's better than any streaming based solution because using this library you don't need to worry about inter process communication and it's a single dependency.

This project was forked by OpenBCI from [Andrey1994/brainflow](https://github.com/Andrey1994/brainflow) and now this is the official version which is supported by OpenBCI and @Andrey1994

## [BrainFlow Docs, Dev and User guides and other information](https://brainflow.readthedocs.io/en/latest/index.html)

## Build status
*Linux(Travis)*:

[![Build Status](https://travis-ci.com/OpenBCI/brainflow.svg?branch=master)](https://travis-ci.com/OpenBCI/brainflow)

*Windows(AppVeyour)*:

[![Build status](https://ci.appveyor.com/api/projects/status/kuuoa32me3253jad/branch/master?svg=true)](https://ci.appveyor.com/project/daniellasry/brainflow/branch/master)

## Shared library methods:
```
int prepare_session (int board_id, const char *port_name);
int start_stream (int buffer_size);
int stop_stream ();
int release_session ();
int get_current_board_data (int num_samples, float *data_buf, double *ts_buf, int *returned_samples);
int get_board_data_count (int *result);
int get_board_data (int data_count, float *data_buf, double *ts_buf);
int set_log_level (int log_level);
```

## Brainflow Bindings
We support bindings for:
* [python](./python-package)
* [java](./java-package/brainflow/)
* [R](./r-package/)
* [CPP](./cpp-package/)
* [Matlab(WIP)](./matlab-package/brainflow/)
* [C#](./csharp-package/brainflow/)

These bindings just call methods from dynamic libraries, if you are interested in other programming languages - feel free to create feature request

## Build instructions

*For now we commit compiled libraries directly to github, so if you don't need Cpp SDK and just want to use binding for your favorite language, you don't need to compile brainflow*

### Compilation

#### Windows
* Install CMAKE>=3.10 (you can install cmake from pip of from [cmake website](https://cmake.org/))
* Install Visual Studio(We just need MSBUILD and you are able to download it without Visual Studio but it's easier to install Visual Studio Community)
```
mkdir build
cd build
# if you install another Visual Studio you need to change the generator in command below, also you can change installation directory(you may skip installation if you don't need Cpp SDK), if you wanna commit compiled libraries you need to specify -DCMAKE_SYSTEM_VERSION=8.1 it's the oldest windows version which we support
cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_SYSTEM_VERSION=8.1 -DCMAKE_INSTALL_PREFIX=..\\installed\\ ..
cmake --build . --target install --config Release
cd ..
```

#### Linux
* Install CMAKE>=3.10

*If you wanna distribute compiled Linux linraries you HAVE to build it inside this [docker continer](https://github.com/Andrey1994/brainflow/blob/master/Docker/Dockerfile) otherwise your libraries will fail on Linux with another glibc version*

Instructions to build and run docker container and [docker getting started guide](https://docs.docker.com/get-started/)
```
cd Docker
docker build .
docker run -it -v %host_path_to_brainflow%:%%path_inside_container% %container_id% /bin/bash
```
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=%install_path% ..
make
# you can skip installation step if you don't need Cpp API
make install
```

### Python

Following commands will install all required packages for you:
```
cd python-package
pip install -e .
```
Or install it from [PYPI](https://pypi.org/project/brainflow/) with:
```
pip install brainflow
```
*For Ganglion(Windows) you may need to install visual c++ redistributable for your PC:*

* [redist_x64](https://aka.ms/vs/16/release/vc_redist.x64.exe)
* [redist_x86](https://aka.ms/vs/16/release/vc_redist.x86.exe)

### R
R package works via [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html) which allows to call Python function directly from R, also it translates all Python classes(even user defined) to the corresponds R constructures.

For now it is not added to the CRAN, so you will have to build this package manually using R-strudio or from command line and install python brainflow bindigs first(I recommend to install it system wide, you can install it to virtualenv but you will need to specify environment variable for reticulate to use this virtualenv)

Usefull Links:
* [Build package with R studio](https://support.rstudio.com/hc/en-us/articles/200486518-Customizing-Package-Build-Options)
* [Reticulate module](https://rstudio.github.io/reticulate/articles/introduction.html)
* You also able to build package from command line, you can check it in .travis.yml file

### Java
Java binding for brainflow is a Maven project to compile it you just need to run:
```
mvn package
```

### C#
You will need to install Nuget packages to build it yourself and compile it using Microsoft Visual Studio

### Cpp
Brainflow is a Cpp library which provides low-level API to read data from OpenBCI, this API used by different bindings and for Cpp there is a high level API as well, you can check high level API classes and functions [here](./cpp-package)

You can find usage example at [cpp-test](./tests/cpp)

You need to compile and install brainflow to use it

## License: 
MIT
