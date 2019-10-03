Supported Boards
=================


Synthetic Board
----------------

This board generates synthetic data and you dont need real hardware to use it.

prepare_session args to select this board:

- board Id: -1
- port: any string

Board Spec:

- num eeg channels: configurable
- num acceleration channels: 3
- sampling rate: configurable
- communication: None


OpenBCI Cyton
--------------

.. image:: https://farm5.staticflickr.com/4817/32567183898_10a4b56659.jpg


Cyton geting started guide from `OpenBCI <https://openbci.github.io/Documentation/docs/01GettingStarted/01-Boards/CytonGS>`_.

prepare_session args to select this board:

- board Id: 0
- port: serial port (e.g. "COM3", "/dev/ttyUSB0", ....)

Board Spec:

- num eeg channels: 8
- num acceleration channels: 3
- sampling rate: 250
- communication: serial port

OpenBCI Ganglion
-----------------

.. image:: https://live.staticflickr.com/65535/48288408326_7f078cd2eb.jpg

Ganglion geting started guide from `OpenBCI <https://openbci.github.io/Documentation/docs/01GettingStarted/01-Boards/GanglionGS>`_.

prepare_session args to select this board:

- board Id: 1
- port: Ganglion Mac address which you can get using Bluetooth Low Energy Explorer application from Microsoft Store on Windows or using hcitool on Unix (e.g. "e6:73:73:18:09:b1"), autodiscovery works, so if you have only one Ganglion you can pass empty string (e.g. "")

Board Spec:

- num eeg channels: 4
- num acceleration channels: 3
- sampling rate: 200
- communication: Bluetooth Low Energy


OpenBCI Cyton Daisy
--------------------

.. image:: https://live.staticflickr.com/65535/48288597712_7ba142797e.jpg

Cyton Daisy geting started guide from `OpenBCI <https://openbci.github.io/Documentation/docs/01GettingStarted/01-Boards/DaisyGS>`_.

prepare_session args to select this board:

- board Id: 2
- port: serial port (e.g. "COM3", "/dev/ttyUSB0", ....)

Board Spec:

- num eeg channels: 16
- num acceleration channels: 3
- sampling rate: 125
- communication: Bluetooth Low Energy
