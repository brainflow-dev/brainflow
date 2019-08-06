# Brainflow

Brainflow is a library intended to obtain, parse and analyze EEG\EMG\ECG data from OpenBCI boards.

Core module of this library is implemented in C\C++ and available for all bindings as a dynamic library. This approach allows to reduce code duplication and simplify maintenance, also it's better than any streaming based solution because using this library you don't need to worry about inter process communication and it's a single dependency.

This project was forked by OpenBCI from [Andrey1994/brainflow](https://github.com/Andrey1994/brainflow) and now this is the official version which is supported by OpenBCI and @Andrey1994

## [BrainFlow Docs, Dev and User guides and other information](https://brainflow.readthedocs.io/en/latest/index.html)

## Build status
*Linux(Travis)*:

[![Build Status](https://travis-ci.com/OpenBCI/brainflow.svg?branch=master)](https://travis-ci.org/OpenBCI/brainflow)

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

## TODO List:
- [ ] add Ganglion
- [ ] add Cython daisy
- [ ] add WiFI shield
- [ ] Support Mac
- [ ] integrate python and R bindings with MNE
- [ ] fix Matlab package
- [ ] code cleanup

## Supported boards:
For now only [OpenBCI Cyton Serial](http://docs.openbci.com/Hardware/02-Cyton) is supported, we are working on other boards
![Cyton](https://farm5.staticflickr.com/4817/32567183898_10a4b56659.jpg)

## Brainflow Bindings
We support bindings for:
* [python](./python-package)
* [java](./java-package/brainflow/)
* [R](./r-package/)
* [CPP](./cpp-package/)
* [Matlab(WIP)](./matlab-package/brainflow/)
* [C#](./csharp-package/brainflow/)

These bindings just call methods from dynamic libraries, if you are interested in other programming languages - feel free to create feature request

### SDK Usage Sample(Python):
```
import argparse
import time
import brainflow


def main ():
    parser = argparse.ArgumentParser ()
    parser.add_argument ('--port', type = str, help  = 'port name', required = True)
    parser.add_argument ('--log', action = 'store_true')
    args = parser.parse_args ()

    if (args.log):
        brainflow.board_shim.BoardShim.enable_board_logger ()
    else:
        brainflow.board_shim.BoardShim.disable_board_logger ()

    board = brainflow.board_shim.BoardShim (brainflow.board_shim.CYTON.board_id, args.port)
    board.prepare_session ()
    board.start_stream ()
    time.sleep (5)
    data = board.get_board_data ()
    board.stop_stream ()
    board.release_session ()

    data_handler = brainflow.preprocess.DataHandler (brainflow.board_shim.CYTON.board_id, numpy_data = data)
    filtered_data = data_handler.preprocess_data (order = 3, start = 1, stop = 50)
    data_handler.save_csv ('results.csv')
    print (filtered_data.head ())
    read_data = brainflow.preprocess.DataHandler (brainflow.board_shim.CYTON.board_id, csv_file = 'results.csv')
    print (read_data.get_data ().head ())


if __name__ == "__main__":
    main ()
```

## Brainflow Emulator (Cyton)

Brainflow emulator for Cyton allows users to run tests without EEG headset.

* On Windows it works using [com0com](http://com0com.sourceforge.net/)
* On Linux it works using pty

You should pass command line to test directly to [cyton_linux.py](./emulator/brainflow_emulator/cyton_linux.py) or to [cyton_windows.py](./emulator/brainflow_emulator/cyton_windows.py), script will add port automatically to provided command line and will start an application


### Example

Install emulator package
```
cd emulator
pip3 install -U .
```
Run test
```
python3 brainflow_emulator/cyton_linux.py python3 /media/sf_folder/brainflow/python-package/examples/brainflow_get_data.py --log --port
INFO:root:Running python3 /media/sf_folder/brainflow/python-package/examples/brainflow_get_data.py --log --port /dev/pts/1
INFO:root:read "b'v'"
INFO:root:read "b'b'"
INFO:root:read "b's'"
INFO:root:stdout is: b'   package_num         eeg1          eeg2  ...    accel2    accel3     timestamp\n0          0.0    59.692543    264.193723  ...  3.526750  1.616875  1.553124e+09\n1          1.0   425.466525   2773.043094  ...  1.519125 -3.823000  1.553124e+09\n2          2.0  1349.970005  12973.129874  ...  2.586875  0.657750  1.553124e+09\n3          3.0  2730.721543  35263.509116  ... -2.776625  1.778750  1.553124e+09\n4          4.0  4333.067974  60181.588357  ...  3.000500 -0.270625  1.553124e+09\n\n[5 rows x 13 columns]\n   package_num         eeg1          eeg2  ...    accel2    accel3     timestamp\n0          0.0    59.692543    264.193723  ...  3.526750  1.616875  1.553124e+09\n1          1.0   425.466525   2773.043094  ...  1.519125 -3.823000  1.553124e+09\n2          2.0  1349.970005  12973.129874  ...  2.586875  0.657750  1.553124e+09\n3          3.0  2730.721543  35263.509116  ... -2.776625  1.778750  1.553124e+09\n4          4.0  4333.067974  60181.588357  ...  3.000500 -0.270625  1.553124e+09\n\n[5 rows x 13 columns]\n'
INFO:root:stderr is: b'[2019-03-20 19:12:10.313] [board_logger] [info] openning port /dev/pts/1: 0\n'
```

Emulator raises *TestFailureError* if test fails

## Brainflow emulator (Ganglion)

To emulate Ganglion device you need to compile mock from GanglionBLEAPI folder and put compiled libraries instead original files like GanglionLibNative64.dll, GanglionLibNative32.dll and the same for Linux\MacOs libraries, rebuild brainflow just to put new libraries to correct locations and run as with real board

## Brainflow Tests

### Instructions to run tests

You need to install emulator package first
```
cd {project_dir}/emulator
python setup.py install
```

#### Run Python tests
```
cd {project_dir}/python-package
pip install -e .
# to test it on Linux machine
python {project_dir}/emulator/brainflow_emulator/cyton_linux.py python {project_dir}/tests/python/brainflow_get_data.py
# to test it on Windows machine
python {project_dir}/emulator/brainflow_emulator/cyton_windows.py python {project_dir}/tests/python/brainflow_get_data.py
```

#### Run Java test
```
cd {project_dir}\java-package
# I dont add jar to repo, so you will need to build it
mvn package
cd {project_dir}/tests/java
javac -classpath {project_dir}\java-package\brainflow\target\brainflow-java-jar-with-dependencies.jar BrainFlowTest.java
python {project_dir}\emulator\brainflow_emulator\cyton_windows.py java -classpath .;{project_dir}\java-package\brainflow\target\brainflow-java-jar-with-dependencies.jar BrainFlowTest
```

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
