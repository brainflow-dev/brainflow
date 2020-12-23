.. _supported-boards-label:

Supported Boards
=================

To create an instance of BoardShim class for your board check required inputs in the table below:

.. csv-table:: Required inputs
   :header: "Board", "Board Id", "BrainFlowInputParams.serial_port", "BrainFlowInputParams.mac_address", "BrainFlowInputParams.ip_address", "BrainFlowInputParams.ip_port", "BrainFlowInputParams.ip_protocol", "BrainFlowInputParams.other_info", "BrainFlowInputParams.timeout", "BrainFlowInputParams.serial_number", "BrainFlowInputParams.file"

   "Playback Board", "BoardIds.PLAYBACK_FILE_BOARD (-3)", "-", "-", "-", "-", "-", "Board Id of master board", "-", "-", "path to file for playback"
   "Streaming Board", "BoardIds.STREAMING_BOARD (-2)", "-", "-", "multicast IP address", "port", "-", "Board Id of master board", "-", "-", "-"
   "Synthetic Board", "BoardIds.SYNTHETIC_BOARD (-1)", "-", "-", "-", "-", "-", "-", "-", "-", "-"
   "Cyton", "BoardIds.CYTON_BOARD (0)", "dongle serial port(COM3, /dev/ttyUSB0, /dev/cu.usbserial-xxxxxx...)", "-", "-", "-", "-", "-", "-", "-", "-"
   "Ganglion", "BoardIds.GANGLION_BOARD (1)", "dongle serial port(COM3, /dev/ttyUSB0...)", "Optional: Ganglion's MAC address", "-", "-", "-", "-", "Timeout for device discovery(default 15sec)", "-", "-"
   "Cyton Daisy", "BoardIds.CYTON_DAISY_BOARD (2)", "dongle serial port(COM3, /dev/ttyUSB0, /dev/cu.usbserial-xxxxxx...)", "-", "-", "-", "-", "-", "-", "-", "-"
   "Ganglion WIFI", "BoardIds.GANGLION_WIFI_BOARD (4)", "-", "-", "WIFI Shield IP(default 192.168.4.1)", "any local port which is free", "-", "-", "Timeout for HTTP response(default 10sec)", "-", "-"
   "Cyton WIFI", "BoardIds.CYTON_WIFI_BOARD (5)", "-", "-", "WIFI Shield IP(default 192.168.4.1)", "any local port which is free", "-", "-", "Timeout for HTTP response(default 10sec)", "-", "-"
   "Cyton Daisy WIFI", "BoardIds.CYTON_DAISY_WIFI_BOARD (6)", "-", "-", "WIFI Shield IP(default 192.168.4.1)", "any local port which is free", "-", "-", "Timeout for HTTP response(default 10sec)", "-", "-"
   "BrainBit", "BoardIds.BRAINBIT_BOARD (7)", "-", "-", "-", "-", "-", "-", "Timeout for device discovery(default 15sec)", "Optional: Serial Number of BrainBit device", "-"
   "Unicorn", "BoardIds.UNICORN_BOARD (8)", "-", "-", "-", "-", "-", "-", "-", "Optional: Serial Number of Unicorn device", "-"
   "CallibriEEG", "BoardIds.CALLIBRI_EEG_BOARD (9)", "-", "-", "-", "-", "-", "Optional: ExternalSwitchInputMioUSB (default is ExternalSwitchInputMioElectrodes)", "Timeout for device discovery(default 15sec)", "-", "-"
   "CallibriEMG", "BoardIds.CALLIBRI_EMG_BOARD (10)", "-", "-", "-", "-", "-", "Optional: ExternalSwitchInputMioUSB (default is ExternalSwitchInputMioElectrodes)", "Timeout for device discovery(default 15sec)", "-", "-"
   "CallibriECG", "BoardIds.CALLIBRI_ECG_BOARD (11)", "-", "-", "-", "-", "-", "Optional: ExternalSwitchInputMioUSB (default is ExternalSwitchInputMioElectrodes)", "Timeout for device discovery(default 15sec)", "-", "-"
   "Notion 1", "BoardIds.NOTION_1_BOARD (13)", "-", "-", "-", "-", "-", "-", "-", "Optional: serial number", "-"
   "Notion 2", "BoardIds.NOTION_2_BOARD (14)", "-", "-", "-", "-", "-", "-", "-", "Optional: serial number", "-"
   "FreeEEG32", "BoardIds.FREEEEG32_BOARD (17)", "dongle serial port", "-", "-", "-", "-", "-", "-", "-", "-"


Playback File Board
---------------------

This board playbacks file recorded using another BrainFlow board.

**It allows you to test signal processing algorithms on real data without device.**

To choose this board in BoardShim constructor please specify:

- board_id: -3
- other_info field of BrainFlowInputParams structure should contain board_id of device used to create playback file
- file field of BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

By default it generates new timestamps and stops at the end of the file. You can override it using commands:

.. code-block:: python

   board.config_board ('loopback_true')
   board.config_board ('loopback_false')
   board.config_board ('new_timestamps')
   board.config_board ('old_timestamps')

In methods like:

.. code-block:: python

   get_eeg_channels (board_id)
   get_emg_channels (board_id)
   get_ecg_channels (board_id)
   # .......


You need to use master board id instead Playback Board Id, because exact data format for playback board is controlled by master board as well as sampling rate.

Board Specs:

- num eeg(emg,...) channels: like in master board
- num acceleration channels: like in master board
- sampling rate: like in master board
- communication: UDP multicast socket to read data from master board

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

To choose this board in BoardShim constructor please specify:

- board_id: -1
- you dont need to set any fields in BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Android

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

`Cyton Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/CytonGS>`_

To choose this board in BoardShim constructor please specify:

- board_id: 0
- serial_port field of BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

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

`Ganglion Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/GanglionGS>`_

**To use Ganglion board you need a** `dongle <https://shop.openbci.com/collections/frontpage/products/ganglion-dongle>`_

To choose this board in BoardShim constructor please specify:

- board_id: 1
- serial_port field of BrainFlowInputParams structure
- mac_address field of BrainFlowInputParams structure, if its empty BrainFlow will try to autodiscover Ganglion
- optional: timeout field of BrainFlowInputParams structure, default is 15sec

To get Ganglion's MAC address you can use:

- Windows: `Bluetooth LE Explorer App <https://www.microsoft.com/en-us/p/bluetooth-le-explorer/9n0ztkf1qd98?activetab=pivot:overviewtab>`_
- Linux: hcitool command

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

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

`CytonDaisy Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/DaisyGS>`_

To choose this board in BoardShim constructor please specify:

- board_id: 2
- serial_port field of BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

Board Spec:

- num eeg(emg,...) channels: 16
- num acceleration channels: 3
- sampling rate: 125
- communication: serial port
- signal gain: 24


Ganglion with WIFI Shield
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48836544227_05059fc450_b.jpg
    :width: 300px
    :height: 300px

`WIFI Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_

`WIFI Shield Programming Guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_

To choose this board in BoardShim constructor please specify:

- board_id: 4
- ip_address field of BrainFlowInputParams structure should contain WiFi Shield Ip address(in direct mode its 192.168.4.1), if it's empty BrainFlow will try to autodiscover WIFI Shield and in case of failure will try to use 192.168.4.1
- ip_port field of BrainFlowInputParams structure should be any local port which is free right now
- optional: timeout field of BrainFlowInputParams structure, default is 10sec

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Android

Board Spec:

- num eeg(emg,...) channels: 4
- num acceleration channels: 3
- sampling rate: 1600
- communication: TCP socket to read data and HTTP to send commands

Cyton with WIFI Shield
~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48836367066_a8c4b6d3be_b.jpg
    :width: 400px
    :height: 325px

`WIFI shield Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_

`WIFI shield Programming Guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_

To choose this board in BoardShim constructor please specify:

- board_id: 5
- ip_address field of BrainFlowInputParams structure should contain WiFi Shield Ip address(in direct mode its 192.168.4.1), if it's empty BrainFlow will try to autodiscover WIFI Shield and in case of failure will try to use 192.168.4.1
- ip_port field of BrainFlowInputParams structure should be any local port which is free right now
- optional: timeout field of BrainFlowInputParams structure, default is 10sec

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Android

Board Spec:

- num eeg(emg,...) channels: 8
- num acceleration channels: 3
- sampling rate: 1000
- communication: TCP socket to read data and HTTP to send commands
- signal gain: 24

CytonDaisy with WIFI Shield
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48843419918_f11c90deb0_k.jpg
    :width: 400px
    :height: 400px

`WIFI Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/docs/01GettingStarted/01-Boards/WiFiGS>`_

`WIFI Shield Programming Guide from OpenBCI <https://docs.openbci.com/docs/05ThirdParty/03-WiFiShield/WiFiProgam>`_

To choose this board in BoardShim constructor please specify:

- board_id: 6
- ip_address field of BrainFlowInputParams structure should contain WiFi Shield Ip address(in direct mode its 192.168.4.1), if it's empty BrainFlow will try to autodiscover WIFI Shield and in case of failure will try to use 192.168.4.1
- ip_port field of BrainFlowInputParams structure should be any local port which is free right now
- optional: timeout field of BrainFlowInputParams structure, default is 10sec

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Android

Board Spec:

- num eeg(emg,...) channels: 16
- num acceleration channels: 3
- sampling rate: 1000
- communication: TCP socket to read data and HTTP to send commands
- signal gain: 24

NeuroMD
----------

BrainBit
~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/49579371806_80b1bffae1.jpg
    :width: 400px
    :height: 400px

`BrainBit website <https://brainbit.com/>`_

To choose this board in BoardShim constructor please specify:

- board_id: 7
- optional: serial_number field of BrainFlowInputParams structure should contain Serial Number of BrainBit device, use it if you have multiple devices
- optional: timeout field of BrainFlowInputParams structure, default is 15sec

Supported platforms:

- Windows >= 10
- MacOS

Board Spec:

- num eeg channels: 4
- num acceleration channels: None
- sampling rate: 250
- communication: Bluetooth Low Energy


Callibri(Yellow)
~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/49906443867_315307d6fc_w.jpg
    :width: 338px
    :height: 400px

`Callibri website <https://callibri.com/>`_

Callibri can be used to record EMG, ECG and EEG, but based on signal type you need to apply different settings for device.

BrainFlow does it for you, so there are:

- CALLIBRI_EEG_BOARD (board_id 9)
- CALLIBRI_EMG_BOARD (board_id 10)
- CALLIBRI_ECG_BOARD (board_id 11)

To choose this board in BoardShim constructor please specify:

- board_id: 9, 10 or 11 based on data type
- optional: to use electrodes connected vis USB write "ExternalSwitchInputMioUSB" to other_info field of BrainFlowInputParams structure
- optional: timeout field of BrainFlowInputParams structure, default is 15sec

Supported platforms:

- Windows >= 10
- MacOS

Board Spec:

- num exg channels: 1
- num acceleration channels: None
- communication: Bluetooth Low Energy


G.TEC
------

Unicorn
~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/49740988577_c54162024d_h.jpg
    :width: 600px
    :height: 450px

`Unicorn website <https://www.unicorn-bi.com/>`_

To choose this board in BoardShim constructor please specify:

- board_id: 8
- optional: serial_number field of BrainFlowInputParams structure should contain Serial Number of BrainBit device, use it if you have multiple devices

Supported platforms:

- Ubuntu 18.04, may work on other Linux OSes, it depends on dynamic library provided by Unicorn
- Windows
- May also work on Raspberry PI, if you replace libunicorn.so by library provided by Unicorn for Raspberry PI

Steps to Setup:

- Connect the dongle
- Make sure that you paired Unicorn device with PC using provided dongle instead built-in Bluetooth

Board Spec:

- num eeg channels: 8
- num acceleration channels: 3
- sampling rate: 250
- communication: Bluetooth Low Energy

Neurosity
----------

Notion 1
~~~~~~~~~

`Notion website <https://neurosity.co/>`_

`Link to Neurosity Tutorial <https://dev.to/neurosity/using-brainflow-with-the-neurosity-headset-2kof>`_

To choose this board in BoardShim constructor please specify:

- board_id: 13
- optional: Serial Number field of BrainFlowInputParams structure, important if you have multiple devices in the same place

Supported platforms:

- Windows
- Linux
- MacOS

*Note: On Windows you may need to disable firewall to allow broadcast messages.*

Board Spec:

- num eeg channels: 8
- sampling rate: 250
- communication: UDP BroadCast

Notion 2
~~~~~~~~~

`Notion website <https://neurosity.co/>`_

`Link to Neurosity Tutorial <https://dev.to/neurosity/using-brainflow-with-the-neurosity-headset-2kof>`_

To choose this board in BoardShim constructor please specify:

- board_id: 14
- optional: Serial Number field of BrainFlowInputParams structure, important if you have multiple devices in the same place

Supported platforms:

- Windows
- Linux
- MacOS

*Note: On Windows you may need to disable firewall to allow broadcast messages.*

Board Spec:

- num eeg channels: 8
- sampling rate: 250
- communication: UDP BroadCast


FreeEEG32
----------

FreeEEG32
~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/50587672267_2f23300f5e_c.jpg
    :width: 400px
    :height: 225px

`CrowdSupply <https://www.crowdsupply.com/neuroidss/freeeeg32>`_

To choose this board in BoardShim constructor please specify:

- board_id: 17
- serial_port field of BrainFlowInputParams structure

Supported platforms:

- Windows
- Linux
- MacOS

Board Spec:

- num eeg channels: 32
- sampling rate: 512
- communication: Serial Port