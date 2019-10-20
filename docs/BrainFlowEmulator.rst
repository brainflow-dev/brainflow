Brainflow Emulator
==================

Brainflow Emulator allows you to run all integration tests for all supported boards without real hardware, our CI uses it for test automation also you can run it on your own PC

Cyton
-------

Cyton emulator simulate COM port using:

- `com0com <http://com0com.sourceforge.net/>`_ for Windows
- pty for Linux and MacOS

You should pass command line to test directly to cyton_linux.py or to cyton_windows.py, script will add port automatically to provided command line and will start an application


Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests for Linux\MacOS and Windows (port argument will be added by Emulator!) ::

    python brainflow_emulator/cyton_linux.py python ../python-package/examples/brainflow_get_data.py --log --board-id 0 --serial-port
    python brainflow_emulator\cyton_windows.py python ..\python-package\examples\brainflow_get_data.py --log --board-id 0 --serial-port

Ganglion
----------

Interaction with Ganglion is implemented in another dynamic library, brainflow loads this library at runtime and calls methods from this library, there is a mock for this library, so you need to compile mock version and replace real library by it. Use Cmake to build mock


NovaXR
-------

NovaXR emulator creates socket server and streams data to brainflow like it's a real board but with much lower sampling rate

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests::

    python brainflow_emulator/novaxr_udp.py python ../python-package/examples/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 3 --ip-protocol 1

Wifi shield based boards
--------------------------

Wifi shield emulator starts http server to read command and creates client socket to stream data

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests for Ganglion, Cyton and Daisy with Wifi Shield::

    python brainflow_emulator/wifi_shield_emulator.py python ../python-package/examples/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 4 --ip-protocol 2 --ip-port 17982
    python brainflow_emulator/wifi_shield_emulator.py python ../python-package/examples/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 5 --ip-protocol 2 --ip-port 17982
    python brainflow_emulator/wifi_shield_emulator.py python ../python-package/examples/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 6 --ip-protocol 2 --ip-port 17982

