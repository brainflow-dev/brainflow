.. _emulator-label:

BrainFlow Emulator
===================

BrainFlow Emulator allows you to run all integration tests for all supported boards without real hardware, our CI uses it for test automation, also you can run it on your own PC.

Streaming Board
-----------------

Streaming Board emulator works using Python binding for BrainFlow, so **you need to install Python binding first.**

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests ::

    python emulator\brainflow_emulator\streaming_board_emulator.py python tests\python\brainflow_get_data.py --log --board-id -2 --ip-address 225.1.1.1 --ip-port 6677 --other-info -1

This emulator uses synthetic board as a master board and ip address and port are hardcoded.

OpenBCI Cyton
--------------

Cyton emulator simulate COM port using:

- `com0com <http://com0com.sourceforge.net/>`_ for Windows
- pty for Linux and MacOS

You should pass test command line directly to cyton_linux.py or to cyton_windows.py, script will add port automatically to provided command line and will start an application.


Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests for Linux\MacOS and Windows (port argument will be added by Emulator!) ::

    python brainflow_emulator/cyton_linux.py python ../tests/python/brainflow_get_data.py --log --board-id 0 --serial-port
    python brainflow_emulator\cyton_windows.py python ..\tests\python\brainflow_get_data.py --log --board-id 0 --serial-port


OpenBCI NovaXR
---------------

NovaXR emulator creates socket server and streams data to BrainFlow like it's a real board but with much lower sampling rate.

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests::

    python brainflow_emulator/novaxr_udp.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 3 --ip-protocol 1

OpenBCI Wifi Shield based boards
----------------------------------

Wifi shield emulator starts http server to read commands and creates client socket to stream data.

Install emulator package::

    cd emulator
    python -m pip install -U .

Run tests for Ganglion, Cyton and Daisy with Wifi Shield::

    python brainflow_emulator/wifi_shield_emulator.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 4 --ip-protocol 2 --ip-port 17982
    python brainflow_emulator/wifi_shield_emulator.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 5 --ip-protocol 2 --ip-port 17982
    python brainflow_emulator/wifi_shield_emulator.py python ../tests/python/brainflow_get_data.py --log --ip-address 127.0.0.1 --board-id 6 --ip-protocol 2 --ip-port 17982

