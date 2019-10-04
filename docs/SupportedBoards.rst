Supported Boards
=================


Synthetic Board
----------------

This board generates synthetic data and you dont need real hardware to use it.

prepare_session args to select this board:

- board Id: -1
- port: any string

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Specs:

- num eeg(emg) channels: configurable
- num acceleration channels: configurable
- sampling rate: configurable
- communication: None


OpenBCI Cyton
--------------

.. image:: https://farm5.staticflickr.com/4817/32567183898_10a4b56659.jpg

`Cyton geting started guide from OpenBCI <https://docs.openbci.com/Tutorials/00-Tutorials>`_.

prepare_session args to select this board:

- board Id: 0
- port: serial port (COM3, /dev/ttyUSB0 ....)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Spec:

- num eeg(emg) channels: 8
- num acceleration channels: 3
- sampling rate: 250
- communication: serial port

OpenBCI Ganglion
-----------------

.. image:: https://live.staticflickr.com/65535/48288408326_7f078cd2eb.jpg

`Ganglion geting started guide from OpenBCI <https://docs.openbci.com/Tutorials/00-Tutorials>`_.

prepare_session args to select this board:

- board Id: 1
- port: for Windows - Ganglion Mac address which you can get using Bluetooth Low Energy Explorer from Microsoft Store (e.g. e6:73:73:18:09:b1) for Windows autodiscovery works so you can pass empty string as well, for Linux/MacOS you need to pass string in format "dongle_port,mac_addr"(e.g./dev/ttyACM0,e6:73:73:18:09:b1) to get mac address on Linux you can use hcitool

Supported platforms:

- Windows >= 10
- Linux
- MacOS

Additional configuration:

- If you use python bindings installed from PYPI on Windows you may need to install `redist_x64 <https://aka.ms/vs/16/release/vc_redist.x64.exe>`_ or `redist_x86 <https://aka.ms/vs/16/release/vc_redist.x86.exe>`_  (but more likely you have it preinstalled)
- For Linux/MacOS you need to use dongle, for Windows dongle is not required if you have Bluetooth on your laptop

Board Spec:

- num eeg(emg) channels: 4
- num acceleration channels: 3
- sampling rate: 200
- communication: Bluetooth Low Energy


OpenBCI Cyton Daisy
--------------------

.. image:: https://live.staticflickr.com/65535/48288597712_7ba142797e.jpg

`Cyton Daisy geting started guide from OpenBCI <https://docs.openbci.com/Tutorials/00-Tutorials>`_.

prepare_session args to select this board:

- board Id: 2
- port: serial port (COM3, /dev/ttyUSB0 ....)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Spec:

- num eeg(emg) channels: 16
- num acceleration channels: 3
- sampling rate: 125
- communication: Bluetooth Low Energy


OpenBCI Ganglion with Wifi Shield
-----------------------------------

.. image:: https://live.staticflickr.com/65535/48836544227_05059fc450_b.jpg

`Wifi shield geting started guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_.

`Wifi shield programming guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_.


API params:

- board Id: 4
- port: shield ip address (192.168.4.1 for direct mode)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS


Board Spec:

- num eeg(emg) channels: 4
- num acceleration channels: 3
- sampling rate: 1600
- communication: tcp socket to read data(local port 17982) and http to send commands



OpenBCI Cyton with Wifi Shield
---------------------------------

.. image:: https://live.staticflickr.com/65535/48836367066_a8c4b6d3be_b.jpg

`Wifi shield geting started guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_.

`Wifi shield programming guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_.

API params:

- board Id: 5
- port: shield ip address (192.168.4.1 for direct mode)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS


Board Spec:

- num eeg(emg) channels: 8
- num acceleration channels: 3
- sampling rate: 1000
- communication: tcp socket to read data(local port 17982) and http to send commands


OpenBCI Cyton Daisy with Wifi Shield
--------------------------------------

.. image:: https://live.staticflickr.com/65535/48843419918_f11c90deb0_k.jpg

`Wifi shield geting started guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_.

`Wifi shield programming guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_.


API params:

- board Id: 6
- port: shield ip address (192.168.4.1 for direct mode)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS


Board Spec:

- num eeg(emg) channels: 16
- num acceleration channels: 3
- sampling rate: 1000
- communication: tcp socket to read data(local port 17982) and http to send commands
