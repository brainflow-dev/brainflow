Supported Boards
=================


Synthetic Board
---------------

API params:

- board Id: -1
- port: any string

System Requirements:

- Windows >= 8.1/Linux/MacOS
- For Python and C# we support x86/x64 libraries, for other bindings x64 only

This board generates synthetic data and you dont need real hardware to use it.

Board Spec:

- num eeg channels: configurable
- num acceleration channels: 3
- sampling rate: configurable
- communication: None


OpenBCI Cyton
--------------

.. image:: https://farm5.staticflickr.com/4817/32567183898_10a4b56659.jpg

API params:

- board Id: 0
- port: serial port (COM3, /dev/ttyUSB0 ....)

Cyton geting started guide from `OpenBCI 
<https://docs.openbci.com/Tutorials/00-Tutorials>`_.

System Requirements:

- Windows >= 8.1/Linux/MacOS
- For Python and C# we support x86/x64 libraries, for other bindings x64 only

Board Spec:

- num eeg channels: 8
- num acceleration channels: 3
- sampling rate: 250
- communication: serial port

OpenBCI Ganglion
----------------

.. image:: https://live.staticflickr.com/65535/48288408326_7f078cd2eb.jpg

API params:

- board Id: 1
- port: for Windows - Ganglion Mac address which you can get using Bluetooth Low Energy Explorer from Microsoft Store (e.g. e6:73:73:18:09:b1) for Windows autodiscovery works so you can pass empty string as well, for Linux/MacOS you need to pass string in format "dongle_port,mac_addr"(e.g./dev/ttyACM0,e6:73:73:18:09:b1)  to get mac address on Linux you can use hcitool

Ganglion geting started guide from `OpenBCI 
<https://docs.openbci.com/Tutorials/00-Tutorials>`_.

System Requirements:

- Windows >= 10/Linux/MacOS
- For Python and C# we support x86/x64 libraries, for other bindings x64 only
- If you use python bindings installed from PYPI on Windows you may need to install `redist_x64 <https://aka.ms/vs/16/release/vc_redist.x64.exe>`_ or `redist_x86 <https://aka.ms/vs/16/release/vc_redist.x86.exe>`_  (but more likely you have it preinstalled)
- For Linux/MacOS you need to use dongle, for Windows dongle is not required if you have Bluetooth on your laptop

Board Spec:

- num eeg channels: 4
- num acceleration channels: 3
- sampling rate: 200
- communication: Bluetooth Low Energy


OpenBCI Cyton Daisy
-------------------

.. image:: https://live.staticflickr.com/65535/48288597712_7ba142797e.jpg

API params:

- board Id: 2
- port: serial port (COM3, /dev/ttyUSB0 ....)

Cyton Daisy geting started guide from `OpenBCI 
<https://docs.openbci.com/Tutorials/00-Tutorials>`_.

System Requirements:

- Windows >= 8.1/Linux/MacOS
- For Python and C# we support x86/x64 libraries, for other bindings x64 only

Board Spec:

- num eeg channels: 16
- num acceleration channels: 3
- sampling rate: 125
- communication: Bluetooth Low Energy
