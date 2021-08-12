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
   "GforcePro", "BoardIds.GFORCE_PRO_BOARD (16)", "-", "-", "-", "-", "-", "-", "-", "-", "-"
   "FreeEEG32", "BoardIds.FREEEEG32_BOARD (17)", "dongle serial port", "-", "-", "-", "-", "-", "-", "-", "-"
   "BrainBitBLED", "BoardIds.BRAINBIT_BLED_BOARD (18)", "dongle serial port", "Optional: MAC address", "-", "-", "-", "-", "-", "-", "-"
   "GforceDual", "BoardIds.GFORCE_DUAL_BOARD (19)", "-", "-", "-", "-", "-", "-", "-", "-", "-"
   "MuseSBLED", "BoardIds.MUSE_S_BLED_BOARD (21)", "dongle serial port", "-", "-", "-", "-", "-", "-", "Optional: device name", "-"
   "Muse2BLED", "BoardIds.MUSE_2_BLED_BOARD (22)", "dongle serial port", "-", "-", "-", "-", "-", "-", "Optional: device name", "-"
   "Crown", "BoardIds.CROWN_BOARD (23)", "-", "-", "-", "-", "-", "-", "-", "Optional: serial number", "-"
   "AntNeuroBoardEE410", "BoardIds.ANT_NEURO_EE_410_BOARD (24)", "-", "-", "-", "-", "-", "-", "-", "-", "-"
   "AntNeuroBoardEE411", "BoardIds.ANT_NEURO_EE_411_BOARD (25)", "-", "-", "-", "-", "-", "-", "-", "-", "-"
   "AntNeuroBoardEE430", "BoardIds.ANT_NEURO_EE_430_BOARD (26)", "-", "-", "-", "-", "-", "-", "-", "-", "-"
   "Enophone", "BoardIds.ENOPHONE_BOARD (37)", "-", "MAC adress", "-", "-", "-", "-", "-", "-", "-"

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
- communication: None

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

`Cyton Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/CytonGS/>`_

To choose this board in BoardShim constructor please specify:

- board_id: 0
- serial_port field of BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

**Also, on Unix-like systems you may need to configure permissions for serial port or run with sudo.**

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

`Ganglion Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/GanglionGS/>`_

**To use Ganglion board you need a** `dongle <https://shop.openbci.com/collections/frontpage/products/ganglion-dongle>`_

**Also, on Unix-like systems you may need to configure permissions for serial port or run with sudo.**

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

`CytonDaisy Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/DaisyGS/>`_

To choose this board in BoardShim constructor please specify:

- board_id: 2
- serial_port field of BrainFlowInputParams structure

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

**Also, on Unix-like systems you may need to configure permissions for serial port or run with sudo.**

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

`WIFI Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/WiFiGS/>`_

`WIFI Shield Programming Guide from OpenBCI <https://docs.openbci.com/ThirdParty/WiFiShield/WiFiProgam/>`_

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

`WIFI shield Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/WiFiGS/>`_

`WIFI shield Programming Guide from OpenBCI <https://docs.openbci.com/ThirdParty/WiFiShield/WiFiProgam/>`_

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

`WIFI Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/WiFiGS/>`_

`WIFI Shield Programming Guide from OpenBCI <https://docs.openbci.com/ThirdParty/WiFiShield/WiFiProgam/>`_

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

BrainBitBLED
~~~~~~~~~~~~~~

This board allows you to use `BLED112 dongle <https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112>`_ instead native API to work with BLE. Unlike original BrainBit libraries it works on Linux and devices like Raspberry Pi.

To choose this board in BoardShim constructor please specify:

- board_id: 18
- serial port field of BrainFlowInputParams structure
- optional: MAC address for your BrainBit device

Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Board Spec:

- num eeg channels: 4
- num acceleration channels: None
- sampling rate: 250
- communication: Bluetooth Low Energy with serial port dongle


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

.. image:: https://live.staticflickr.com/65535/51302873810_715f51b408.jpg
    :width: 500px
    :height: 353px

`Neurosity website <https://neurosity.co/>`_

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

.. image:: https://live.staticflickr.com/65535/51302045358_b375380804.jpg
    :width: 500px
    :height: 353px


`Neurosity website <https://neurosity.co/>`_

`Link to Neurosity Tutorial <https://dev.to/neurosity/using-brainflow-with-the-neurosity-headset-2kof>`_

To choose this board in BoardShim constructor please specify:

- board_id: 23
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

Crown
~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51301110182_9d05de3948.jpg
    :width: 500px
    :height: 353px

`Neurosity website <https://neurosity.co/>`_

`Link to Neurosity Tutorial <https://dev.to/neurosity/using-brainflow-with-the-neurosity-headset-2kof>`_

To choose this board in BoardShim constructor please specify:

- board_id: 23
- optional: Serial Number field of BrainFlowInputParams structure, important if you have multiple devices in the same place

Supported platforms:

- Windows
- Linux
- MacOS

*Note: On Windows you may need to disable firewall to allow broadcast messages.*

Board Spec:

- num eeg channels: 8
- sampling rate: 256
- communication: UDP BroadCast

OYMotion
---------

gForcePro ArmBand
~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/50760349443_368326974c_o.jpg
    :width: 484px
    :height: 430px

`OYMotion website <http://www.oymotion.com/en/product32/149>`_

To choose this board in BoardShim constructor please specify:

- board_id: 16

Supported platforms:

- Windows

*Note: Unlike other boards it returns ADC value instead uV.*

Board Spec:

- num emg channels: 8
- sampling rate: 500

gForceDual ArmBand
~~~~~~~~~~~~~~~~~~~

`OYMotion website <http://www.oymotion.com/en>`_

To choose this board in BoardShim constructor please specify:

- board_id: 19

Supported platforms:

- Windows

*Note: Unlike other boards it returns ADC value instead uV.*

Board Spec:

- num emg channels: 2
- sampling rate: 500

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

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

Supported platforms:

- Windows
- Linux
- MacOS

Board Spec:

- num eeg channels: 32
- sampling rate: 512
- communication: Serial Port

Muse
------

Muse S BLED
~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51249005962_026502fee0.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

To use this board you need to get `BLED112 dongle <https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112>`_.

**Also, on Unix-like systems you may need to configure permissions for serial port or run with sudo.**

To choose this board in BoardShim constructor please specify:

- board_id: 21
- serial port field of BrainFlowInputParams structure
- optional: serial number(device name)

Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Board Spec:

- num eeg channels: 4
- num acceleration channels: 3
- sampling rate: 250
- communication: Bluetooth Low Energy with serial port dongle

Muse 2 BLED
~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51250482419_32ce8454dd.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

To use this board you need to get `BLED112 dongle <https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112>`_.

**Also, on Unix-like systems you may need to configure permissions for serial port or run with sudo.**

To choose this board in BoardShim constructor please specify:

- board_id: 22
- serial port field of BrainFlowInputParams structure
- optional: serial number(device name)

Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Board Spec:

- num eeg channels: 4
- num acceleration channels: 3
- sampling rate: 250
- communication: Bluetooth Low Energy with serial port dongle

Ant Neuro
----------

Ant Neuro EE 410
~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51331462280_580d890535.jpg
    :width: 500px
    :height: 490px

`Ant Website <https://www.ant-neuro.com/products>`_

To choose this board in BoardShim constructor please specify:

- board_id: 24

Supported platforms:

- Windows
- Linux

Board Spec:

- num eeg channels: 8
- sampling rate: 2000

Ant Neuro EE 411
~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51331191524_5cea43c2f2.jpg
    :width: 500px
    :height: 399px

`Ant Website <https://www.ant-neuro.com/products>`_

To choose this board in BoardShim constructor please specify:

- board_id: 25

Supported platforms:

- Windows
- Linux

Board Spec:

- num eeg channels: 8
- sampling rate: 2000

Ant Neuro EE 430
~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51329836457_6438dce763.jpg
    :width: 500px
    :height: 438px

`Ant Website <https://www.ant-neuro.com/products>`_

To choose this board in BoardShim constructor please specify:

- board_id: 26

Supported platforms:

- Windows
- Linux

Board Spec:

- num eeg channels: 8
- sampling rate: 512

Enophone
---------

Enophone Headphones
~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51374388843_f60c07991e.jpg
    :width: 401px
    :height: 500px

`Enophone website <https://enophone.com/enophones/>`_

To choose this board in BoardShim constructor please specify:

- board_id: 37
- mac address field of BrainFlowInputParams structure

Supported platforms:

- Windows
- Linux
- MacOS

Steps to find MAC address:

- On Windows: open device manager, navigate to enophone device, click properties and select Bluetooth Address
- On Linux: install bluez-utils and run :code:`bluetoothctl paired-devices`

Board Spec:

- num emg channels: 4
- sampling rate: 250
- communication: Bluetooth
