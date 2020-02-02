.. _supported-boards-label:

Supported Boards
=================

To create an instance of BoardShim class for your board check required inputs in the table below:

.. csv-table:: Required inputs
   :header: "Board", "Board Id", "BrainFlowInputParams.serial_port", "BrainFlowInputParams.mac_address", "BrainFlowInputParams.ip_address", "BrainFlowInputParams.ip_port", "BrainFlowInputParams.ip_protocol", "BrainFlowInputParams.other_info"

   "Streaming Board", "BoardIds.STREAMING_BOARD (-2)", "-", "-", "multicast IP address", "port", "-", "Board Id of master board"
   "Synthetic Board", "BoardIds.SYNTHETIC_BOARD (-1)", "-", "-", "-", "-", "-", "-"
   "Cyton", "BoardIds.CYTON_BOARD (0)", "dongle serial port(COM3, /dev/ttyUSB0, /dev/cu.usbserial-xxxxxx...)", "-", "-", "-", "-", "-"
   "Ganglion", "BoardIds.GANGLION_BOARD (1)", "dongle serial port(COM3, /dev/ttyUSB0...)", "Optional: Ganglion's MAC address", "-", "-", "-", "-"
   "Cyton Daisy", "BoardIds.CYTON_DAISY_BOARD (2)", "dongle serial port(COM3, /dev/ttyUSB0, /dev/cu.usbserial-xxxxxx...)", "-", "-", "-", "-", "-"
   "Ganglion Wifi", "BoardIds.GANGLION_WIFI_BOARD (4)", "-", "-", "Wifi Shield IP(default 192.168.4.1)", "any local port which is free", "-", "-"
   "Cyton Wifi", "BoardIds.CYTON_WIFI_BOARD (5)", "-", "-", "Wifi Shield IP(default 192.168.4.1)", "any local port which is free", "-", "-"
   "Cyton Daisy Wifi", "BoardIds.CYTON_DAISY_WIFI_BOARD (6)", "-", "-", "Wifi Shield IP(default 192.168.4.1)", "any local port which is free", "-", "-"

Streaming Board
------------------

BrainFlow's boards can stream data to different destinations like file, socket and so on. This board acts like a consumer for data streamed from the main process.

**To use it in the first process you should call:**

.. code-block:: python

    # choose any valid multicast address(from "224.0.0.0" to "239.255.255.255") and port
    start_stream (450000, 'streaming_board://225.1.1.1:6677')

**In the second process please specify:**

- board_id: -2
- ip_address field of BrainFlowInputParams structure, for example above it's 225.1.1.1
- ip_port field of BrainFlowInputParams structure, for example above it's 6677
- other_info field of BrainFlowInputParams structure, write there board_id for a board which acts like data provider(master board)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

In methods like:

.. code-block:: python

   get_eeg_channels (board_id)
   get_emg_channels (board_id)
   get_ecg_channels (board_id)
   # .......

You need to use master board id instead Streaming Board Id, because exact data format for streaming board is controlled by master board as well as sampling rate.

Board Specs:

- num eeg(emg,...) channels: like in master board
- num acceleration channels: like in master board
- sampling rate: like in master board
- communication: UDP multicast socket to read data from master board

Synthetic Board
----------------

This board generates synthetic data and you dont need real hardware to use it.

**It can be extremely useful during development.**

**To choose this board in BoardShim constructor please specify:**

- board_id: -1
- you dont need to set any fields in BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Specs:

- num eeg(emg,...) channels: 8
- num acceleration channels: 3
- sampling rate: 256
- communication: None

OpenBCI
--------

Cyton
~~~~~~~

.. image:: https://i.ibb.co/cNj9pyf/Cyton.jpg
    :width: 200px
    :height: 200px

`Cyton Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/CytonGS>`_.

**To choose this board in BoardShim constructor please specify:**

- board_id: 0
- serial_port field of BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Spec:

- num eeg(emg,...) channels: 8
- num acceleration channels: 3
- sampling rate: 250
- communication: serial port
- signal gain: 24

Ganglion
~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48288408326_7f078cd2eb.jpg
    :width: 400px
    :height: 230px

`Ganglion Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/GanglionGS>`_.

**To use Ganglion board you need a** `dongle <https://shop.openbci.com/collections/frontpage/products/ganglion-dongle>`_.

**To choose this board in BoardShim constructor please specify:**

- board_id: 1
- serial_port field of BrainFlowInputParams structure
- mac_address field of BrainFlowInputParams structure, if its empty BrainFlow will try to autodiscover Ganglion

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Spec:

- num eeg(emg,...) channels: 4
- num acceleration channels: 3
- sampling rate: 200
- communication: Bluetooth Low Energy behind serial port from the dongle

Cyton Daisy
~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48288597712_7ba142797e.jpg
    :width: 400px
    :height: 394px

`CytonDaisy Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/DaisyGS>`_.

**To choose this board in BoardShim constructor please specify:**

- board_id: 2
- serial_port field of BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Spec:

- num eeg(emg,...) channels: 16
- num acceleration channels: 3
- sampling rate: 125
- communication: serial port
- signal gain: 24


Ganglion with WiFi Shield
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48836544227_05059fc450_b.jpg
    :width: 300px
    :height: 300px

`WiFi Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_.

`WiFi Shield Programming Guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_.

**To choose this board in BoardShim constructor please specify:**

- board_id: 4
- ip_address field of BrainFlowInputParams structure should contain WiFi Shield Ip address(in direct mode its 192.168.4.1)
- ip_port field of BrainFlowInputParams structure should be any local port which is free right now

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Spec:

- num eeg(emg,...) channels: 4
- num acceleration channels: 3
- sampling rate: 1600
- communication: TCP socket to read data and HTTP to send commands

Cyton with WiFi Shield
~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48836367066_a8c4b6d3be_b.jpg
    :width: 400px
    :height: 325px

`WiFi shield Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_.

`WiFi shield Programming Guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_.

**To choose this board in BoardShim constructor please specify:**

- board_id: 5
- ip_address field of BrainFlowInputParams structure should contain WiFi Shield Ip address(in direct mode its 192.168.4.1)
- ip_port field of BrainFlowInputParams structure should be any local port which is free right now

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS


Board Spec:

- num eeg(emg,...) channels: 8
- num acceleration channels: 3
- sampling rate: 1000
- communication: TCP socket to read data and HTTP to send commands
- signal gain: 24

CytonDaisy with WiFi Shield
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48843419918_f11c90deb0_k.jpg
    :width: 400px
    :height: 400px

`WiFi Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_.

`WiFi Shield Programming Guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_.

**To choose this board in BoardShim constructor please specify:**

- board_id: 6
- ip_address field of BrainFlowInputParams structure should contain WiFi Shield Ip address(in direct mode its 192.168.4.1)
- ip_port field of BrainFlowInputParams structure should be any local port which is free right now

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

Board Spec:

- num eeg(emg,...) channels: 16
- num acceleration channels: 3
- sampling rate: 1000
- communication: TCP socket to read data and HTTP to send commands
- signal gain: 24
