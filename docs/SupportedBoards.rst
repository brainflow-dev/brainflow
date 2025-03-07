.. _supported-boards-label:

Supported Boards
=================

BrainFlow Dummy Boards
------------------------

Playback File Board
~~~~~~~~~~~~~~~~~~~~~

This board playbacks files recorded using other BrainFlow boards.

**It allows you to test signal processing algorithms on real data without device.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.PLAYBACK_FILE_BOARD`
- :code:`master_board`, it should contain board ID of the device used to create playback files
- :code:`file`, it should contain full path to recorded file
- *optional:* :code:`file_aux`, use it if your master board has auxiliary preset
- *optional:* :code:`file_anc`, use it if your master board has ancillary preset

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.file = "streamer_default.csv"
    params.file_aux = "streamer_aux.csv"
    params.master_board = BoardIds.SYNTHETIC_BOARD
    board = BoardShim(BoardIds.PLAYBACK_FILE_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi

By default it generates new timestamps and stops at the end of the file. You can override such behavior using commands:

.. code-block:: python

    board.config_board ("loopback_true")
    board.config_board ("loopback_false")
    board.config_board ("new_timestamps")
    board.config_board ("old_timestamps")

In methods like:

.. code-block:: python

    get_eeg_channels (board_id)
    get_emg_channels (board_id)
    get_ecg_channels (board_id)
    # .......


You need to use master board id instead Playback Board Id, because exact data format for playback board is controlled by master board as well as sampling rate.

Streaming Board
~~~~~~~~~~~~~~~~~

BrainFlow boards can stream data to different destinations like file, socket, etc directly from BrainFlow. This board acts like a consumer for data streamed from the main process.

To use it in the first process(master process, data provider) you should call:

.. code-block:: python

    # choose any valid multicast address(from "224.0.0.0" to "239.255.255.255") and port
    add_streamer ("streaming_board://225.1.1.1:6677", BrainFlowPresets.DEFAULT_PRESET)

In the second process you shoud create Streaming board instance and this process will act as a data consumer.

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.STREAMING_BOARD`
- :code:`ip_address`, for example above it's 225.1.1.1
- :code:`ip_port`, for example above it's 6677
- :code:`master_board`, it should contain board ID of the device used to create playback files
- *optional:* :code:`ip_address_aux`, use it if your master board has auxiliary preset
- *optional:* :code:`ip_port_aux`, use it if your master board has auxiliary preset
- *optional:* :code:`ip_address_anc`, use it if your master board has ancillary preset
- *optional:* :code:`ip_port_anc`, use it if your master board has ancillary preset

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.ip_port = 6677
    params.ip_port_aux = 6678
    params.ip_address = "225.1.1.1"
    params.ip_address_aux = "225.1.1.1"
    params.master_board = BoardIds.SYNTHETIC_BOARD
    board = BoardShim(BoardIds.STREAMING_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi
- Android

In methods like:

.. code-block:: python

   get_eeg_channels (board_id)
   get_emg_channels (board_id)
   get_ecg_channels (board_id)
   # .......

You need to use master board id instead Streaming Board Id, because exact data format for streaming board is controlled by master board as well as sampling rate.

If you have problems on Windows try to disable virtual box network adapter and firewall. More info can be found `here <https://serverfault.com/a/750820>`_.

Synthetic Board
~~~~~~~~~~~~~~~~

This board generates synthetic data and you dont need real hardware to use it.

**It can be extremely useful during development.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.SYNTHETIC_BOARD`

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.SYNTHETIC_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi
- Android

OpenBCI
--------

Cyton
~~~~~~~

.. image:: https://i.ibb.co/cNj9pyf/Cyton.jpg
    :width: 200px
    :height: 200px

`Cyton Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/CytonGS/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.CYTON_BOARD`
- :code:`serial_port`, e.g. COM3, /dev/ttyUSB0, etc

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM3"
    board = BoardShim(BoardIds.CYTON_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

Ganglion
~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48288408326_7f078cd2eb.jpg
    :width: 400px
    :height: 230px

`Ganglion Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/GanglionGS/>`_

**To use Ganglion board you need a** `dongle <https://shop.openbci.com/collections/frontpage/products/ganglion-dongle>`_

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

**Also, for Macbooks without USB ports you may need to use specific USBC-USB dongles, some of them may lead to slow data streaming.**

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.GANGLION_BOARD`
- :code:`serial_port`, e.g. COM4, /dev/ttyACM0, etc
- *optoinal:* :code:`mac_address`, if not provided BrainFlow will try to autodiscover the device
- *optional:* :code:`timeout`, timeout for device discovery, default is 15sec

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM4"
    board = BoardShim(BoardIds.GANGLION_BOARD, params)

To get Ganglion's MAC address you can use:

- Windows: `Bluetooth LE Explorer App <https://www.microsoft.com/en-us/p/bluetooth-le-explorer/9n0ztkf1qd98?activetab=pivot:overviewtab>`_
- Linux: hcitool command

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi

Ganglion Native
~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48288408326_7f078cd2eb.jpg
    :width: 400px
    :height: 230px

`Ganglion Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/GanglionGS/>`_

Unlike Ganglion board this BrainFlow board does not use BLED112 dongle, so you need to have BLE support on your device in order to use it.

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.GANGLION_NATIVE_BOARD`
- *optoinal:* :code:`mac_address`, if not provided BrainFlow will try to autodiscover the device
- *optoinal:* :code:`serial_number`, if not provided BrainFlow will try to autodiscover the device

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.GANGLION_NATIVE_BOARD, params)

To get Ganglion's MAC address or device name you can use:

- Windows: `Bluetooth LE Explorer App <https://www.microsoft.com/en-us/p/bluetooth-le-explorer/9n0ztkf1qd98?activetab=pivot:overviewtab>`_
- Linux: hcitool command

Supported platforms:

- Windows 10.0.19041.0+
- MacOS 10.15+, 12.0 to 12.2 have known issues while scanning, you need to update to 12.3+. On MacOS 12+ you may need to configure Bluetooth permissions for your appication
- Linux, compilation from source code probably will be needed
- Devices like Raspberry Pi

Cyton Daisy
~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48288597712_7ba142797e.jpg
    :width: 400px
    :height: 394px

`CytonDaisy Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/DaisyGS/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.CYTON_DAISY_BOARD`
- :code:`serial_port`, e.g. COM3, /dev/ttyUSB0, etc

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM3"
    board = BoardShim(BoardIds.CYTON_DAISY_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

Ganglion with WIFI Shield
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48836544227_05059fc450_b.jpg
    :width: 300px
    :height: 300px

`WIFI Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/WiFiGS/>`_

`WIFI Shield Programming Guide from OpenBCI <https://docs.openbci.com/ThirdParty/WiFiShield/WiFiProgam/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.GANGLION_WIFI_BOARD`
- :code:`ip_port`, any local port which is currently free, e.g. 6789
- *optional:* :code:`ip_address`, ip address of WIFI Shield, in direct mode it's 192.168.4.1. If not provided BrainFlow will try to use SSDP for discovery
- *optional:* :code:`timeout`, timeout for device discovery, default is 10sec

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.ip_port = 6987
    params.ip_address = "192.168.4.1"
    board = BoardShim(BoardIds.GANGLION_WIFI_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Android

Cyton with WIFI Shield
~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48836367066_a8c4b6d3be_b.jpg
    :width: 400px
    :height: 325px

`WIFI shield Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/WiFiGS/>`_

`WIFI shield Programming Guide from OpenBCI <https://docs.openbci.com/ThirdParty/WiFiShield/WiFiProgam/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.CYTON_WIFI_BOARD`
- :code:`ip_port`, any local port which is currently free, e.g. 6789
- *optional:* :code:`ip_address`, ip address of WIFI Shield, in direct mode it's 192.168.4.1. If not provided BrainFlow will try to use SSDP for discovery
- *optional:* :code:`timeout`, timeout for device discovery, default is 10sec

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.ip_port = 6987
    params.ip_address = "192.168.4.1"
    board = BoardShim(BoardIds.CYTON_WIFI_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi
- Android

CytonDaisy with WIFI Shield
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/48843419918_f11c90deb0_k.jpg
    :width: 400px
    :height: 400px

`WIFI Shield Getting Started Guide from OpenBCI <https://docs.openbci.com/GettingStarted/Boards/WiFiGS/>`_

`WIFI Shield Programming Guide from OpenBCI <https://docs.openbci.com/ThirdParty/WiFiShield/WiFiProgam/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.CYTON_DAISY_WIFI_BOARD`
- :code:`ip_port`, any local port which is currently free, e.g. 6789
- *optional:* :code:`ip_address`, ip address of WIFI Shield, in direct mode it's 192.168.4.1. If not provided BrainFlow will try to use SSDP for discovery
- *optional:* :code:`timeout`, timeout for device discovery, default is 10sec

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.ip_port = 6987
    params.ip_address = "192.168.4.1"
    board = BoardShim(BoardIds.CYTON_DAISY_WIFI_BOARD, params)

Supported platforms:

- Windows >= 8.1
- Linux
- MacOS
- Devices like Raspberry Pi
- Android

NeuroMD
----------

BrainBit
~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/49579371806_80b1bffae1.jpg
    :width: 400px
    :height: 400px

`BrainBit website <https://brainbit.com/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.BRAINBIT_BOARD`
- *optional:* :code:`serial_number`, serial number of device, usually it's printed on the headset. Important if you have multiple devices in the same place
- *optional:* :code:`timeout`, timeout for device discovery, default is 15sec

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.BRAINBIT_BOARD, params)

Supported platforms:

- Windows >= 10
- MacOS

Available commands for :code:`config_board`:

- CommandStartSignal
- CommandStopSignal
- CommandStartResist
- CommandStopResist

BrainBitBLED
~~~~~~~~~~~~~~

This board allows you to use `BLED112 dongle <https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112>`_ instead native API to work with BLE. Unlike original BrainBit libraries it works on Linux and devices like Raspberry Pi.

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.BRAINBIT_BLED_BOARD`
- :code:`serial port`, e.g. COM4, /dev/ttyACM0
- *optional:* :code:mac_address`, mac address of BrainBit device, important if you have multiple devices in the same place

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM4"
    board = BoardShim(BoardIds.BRAINBIT_BLED_BOARD, params)

To get BrainBit's MAC address you can use:

- Windows: `Bluetooth LE Explorer App <https://www.microsoft.com/en-us/p/bluetooth-le-explorer/9n0ztkf1qd98?activetab=pivot:overviewtab>`_
- Linux: hcitool command

Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Callibri(Yellow)
~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/49906443867_315307d6fc_w.jpg
    :width: 338px
    :height: 400px

`Callibri website <https://callibri.com/>`_

Callibri can be used to record EMG, ECG and EEG, but based on signal type you need to apply different settings for device.

BrainFlow does it for you, so there are:

- :code:`BoardIds.CALLIBRI_EEG_BOARD`
- :code:`BoardIds.CALLIBRI_EMG_BOARD`
- :code:`BoardIds.CALLIBRI_ECG_BOARD`

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.CALLIBRI_EEG_BOARD`
- *optional:* :code:`other_info`, to use electrodes connected vis USB write "ExternalSwitchInputMioUSB" to this field
- *optional:* :code:`timeout`, timeout for device discovery, default is 15sec

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.CALLIBRI_EEG_BOARD, params)

Supported platforms:

- Windows >= 10
- MacOS

G.TEC
------

Unicorn
~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/49740988577_c54162024d_h.jpg
    :width: 600px
    :height: 450px

`Unicorn website <https://www.unicorn-bi.com/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.UNICORN_BOARD`
- *optional:* :code:`serial_number`, important if you have multiple devices in the same place

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.UNICORN_BOARD, params)

Supported platforms:

- Ubuntu 18.04, may work on other Linux OSes, it depends on dynamic library provided by Unicorn
- Windows
- May also work on Raspberry PI, if you replace libunicorn.so by library provided by Unicorn for Raspberry PI

Steps to Setup:

- Connect the dongle
- Make sure that you paired Unicorn device with PC using provided dongle instead built-in Bluetooth

Neurosity
----------

Notion 1
~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51302873810_715f51b408.jpg
    :width: 500px
    :height: 353px

`Neurosity website <https://neurosity.co/>`_

`Link to Neurosity Tutorial <https://dev.to/neurosity/using-brainflow-with-the-neurosity-headset-2kof>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.NOTION_1_BOARD`
- *optional:* :code:`serial_number` important if you have multiple devices in the same place

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.NOTION_1_BOARD, params)

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi

*Note: On Windows you may need to disable firewall to allow broadcast messages. And since the device uses broadcasting it may not work in university network.*

Notion 2
~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51302045358_b375380804.jpg
    :width: 500px
    :height: 353px


`Neurosity website <https://neurosity.co/>`_

`Link to Neurosity Tutorial <https://dev.to/neurosity/using-brainflow-with-the-neurosity-headset-2kof>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.NOTION_2_BOARD`
- *optional:* :code:`serial_number` important if you have multiple devices in the same place

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.NOTION_2_BOARD, params)

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi

*Note: On Windows you may need to disable firewall to allow broadcast messages. And since the device uses broadcasting it may not work in university network.*

Crown
~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51301110182_9d05de3948.jpg
    :width: 500px
    :height: 353px

`Neurosity website <https://neurosity.co/>`_

`Link to Neurosity Tutorial <https://dev.to/neurosity/using-brainflow-with-the-neurosity-headset-2kof>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.CROWN_BOARD`
- *optional:* :code:`serial_number` important if you have multiple devices in the same place

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.CROWN_BOARD, params)

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi

*Note: On Windows you may need to disable firewall to allow broadcast messages. And since the device uses broadcasting it may not work in university network.*

OYMotion
---------

gForcePro ArmBand
~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/50760349443_368326974c_o.jpg
    :width: 484px
    :height: 430px

`OYMotion website <http://www.oymotion.com/en/product32/149>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.GFORCE_PRO_BOARD`

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.GFORCE_PRO_BOARD, params)

Supported platforms:

- Windows

*Note: Unlike other boards it returns ADC values instead uV.*

gForceDual ArmBand
~~~~~~~~~~~~~~~~~~~

`OYMotion website <http://www.oymotion.com/en>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.GFORCE_DUAL_BOARD`

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.GFORCE_DUAL_BOARD, params)

Supported platforms:

- Windows

*Note: Unlike other boards it returns ADC values instead uV.*

FreeEEG
--------

FreeEEG32
~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/50587672267_2f23300f5e_c.jpg
    :width: 400px
    :height: 225px

`CrowdSupply <https://www.crowdsupply.com/neuroidss/freeeeg32>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.FREEEEG32_BOARD`
- :code:`serial_port`, e.g. COM3

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM3"
    board = BoardShim(BoardIds.FREEEEG32_BOARD, params)

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi

FreeEEG128
~~~~~~~~~~

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.FREEEEG128_BOARD`
- :code:`serial_port`, e.g. COM6

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM6"
    board = BoardShim(BoardIds.FREEEEG128_BOARD, params)

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi


Muse
------

Muse S BLED
~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51249005962_026502fee0.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

To use this board you need to get `BLED112 dongle <https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112>`_.

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.MUSE_S_BLED_BOARD`
- :code:`serial_port`, e.g. COM3, /dev/ttyACM0
- *optional:* :code:`serial_number`, device name, can be printed on the Muse device or discoovered via mobile apps

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM3"
    board = BoardShim(BoardIds.MUSE_S_BLED_BOARD, params)

Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data, to enable 5th EEG channel use :code:`board.config_board("p50")`
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data, enabled by default
- :code:`BrainFlowPresets.ANCILLARY_PRESET`, it contains PPG data, to enable it use :code:`board.config_board("p61")`


Muse 2 BLED
~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51250482419_32ce8454dd.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

To use this board you need to get `BLED112 dongle <https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112>`_.

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.MUSE_2_BLED_BOARD`
- :code:`serial_port`, e.g. COM3, /dev/ttyACM0
- *optional:* :code:`serial_number`, device name, can be printed on the Muse device or discoovered via mobile apps

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM3"
    board = BoardShim(BoardIds.MUSE_2_BLED_BOARD, params)


Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data, to enable 5th EEG channel use :code:`board.config_board("p50")`
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data, enabled by default
- :code:`BrainFlowPresets.ANCILLARY_PRESET`, it contains PPG data, to enable it use :code:`board.config_board("p50")`. It also enables 5th channel for EEG


Muse 2016 BLED
~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51854219574_24c42b30d9_w.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

To use this board you need to get `BLED112 dongle <https://www.silabs.com/wireless/bluetooth/bluegiga-low-energy-legacy-modules/device.bled112>`_.

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.MUSE_2016_BLED_BOARD`
- :code:`serial_port`, e.g. COM3, /dev/ttyACM0
- *optional:* :code:`serial_number`, device name, can be printed on the Muse device or discoovered via mobile apps

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM3"
    board = BoardShim(BoardIds.MUSE_2016_BLED_BOARD, params)

Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data, enabled by default

Muse S
~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51249005962_026502fee0.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

.. compound::

    On Linux systems you may need to install `libdbus` and we recommend to compile BrainFlow from the source code: ::

        sudo apt-get install libdbus-1-dev # for ubuntu
        sudo yum install dbus-devel # for centos
        python3 tools/build.py --ble # to compile

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.MUSE_S_BOARD`
- *optional:* :code:`mac_address`, mac address of the device to connect
- *optional:* :code:`serial_number`, device name, can be printed on the Muse device or discoovered via mobile apps

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.MUSE_S_BOARD, params)

Supported platforms:

- Windows 10.0.19041.0+
- MacOS 10.15+, 12.0 to 12.2 have known issues while scanning, you need to update to 12.3+. On MacOS 12+ you may need to configure Bluetooth permissions for your appication
- Linux, compilation from source code probably will be needed
- Devices like Raspberry Pi

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data, to enable 5th EEG channel use :code:`board.config_board("p50")`
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data, enabled by default
- :code:`BrainFlowPresets.ANCILLARY_PRESET`, it contains PPG data, to enable it use :code:`board.config_board("p61")`


Muse 2
~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51250482419_32ce8454dd.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

.. compound::

    On Linux systems you may need to install `libdbus` and we recommend to compile BrainFlow from the source code: ::

        sudo apt-get install libdbus-1-dev # for ubuntu
        sudo yum install dbus-devel # for centos
        python3 tools/build.py --ble # to compile

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.MUSE_2_BOARD`
- *optional:* :code:`mac_address`, mac address of the device to connect
- *optional:* :code:`serial_number`, device name, can be printed on the Muse device or discoovered via mobile apps

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.MUSE_2_BOARD, params)

Supported platforms:

- Windows 10.0.19041.0+
- MacOS 10.15+, 12.0 to 12.2 have known issues while scanning, you need to update to 12.3+. On MacOS 12+ you may need to configure Bluetooth permissions for your appication
- Linux, compilation from source code probably will be needed
- Devices like Raspberry Pi

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data, to enable 5th EEG channel use :code:`board.config_board("p50")`
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data, enabled by default
- :code:`BrainFlowPresets.ANCILLARY_PRESET`, it contains PPG data, to enable it use :code:`board.config_board("p50")`. It also enables 5th channel for EEG


Muse 2016
~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51854219574_24c42b30d9_w.jpg
    :width: 350px
    :height: 350px

`Muse Website <https://choosemuse.com/>`_

.. compound::

    On Linux systems you may need to install `libdbus` and we recommend to compile BrainFlow from the source code: ::

        sudo apt-get install libdbus-1-dev # for ubuntu
        sudo yum install dbus-devel # for centos
        python3 tools/build.py --ble # to compile

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.MUSE_2016_BOARD`
- *optional:* :code:`mac_address`, mac address of the device to connect
- *optional:* :code:`serial_number`, device name, can be printed on the Muse device or discoovered via mobile apps

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.MUSE_2016_BOARD, params)

Supported platforms:

- Windows 10.0.19041.0+
- MacOS 10.15+, 12.0 to 12.2 have known issues while scanning, you need to update to 12.3+. On MacOS 12+ you may need to configure Bluetooth permissions for your appication
- Linux, compilation from source code probably will be needed
- Devices like Raspberry Pi

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data, enabled by default

Ant Neuro
----------

.. image:: https://live.staticflickr.com/65535/51331462280_580d890535.jpg
    :width: 500px
    :height: 490px

`Ant Website <https://www.ant-neuro.com/products>`_

Ant Neuro has many devices and all of them are supported by BrainFlow:

- :code:`ANT_NEURO_EE_410_BOARD`
- :code:`ANT_NEURO_EE_411_BOARD`
- :code:`ANT_NEURO_EE_430_BOARD`
- :code:`ANT_NEURO_EE_211_BOARD`
- :code:`ANT_NEURO_EE_212_BOARD`
- :code:`ANT_NEURO_EE_213_BOARD`
- :code:`ANT_NEURO_EE_214_BOARD`
- :code:`ANT_NEURO_EE_215_BOARD`
- :code:`ANT_NEURO_EE_221_BOARD`
- :code:`ANT_NEURO_EE_222_BOARD`
- :code:`ANT_NEURO_EE_223_BOARD`
- :code:`ANT_NEURO_EE_224_BOARD`
- :code:`ANT_NEURO_EE_225_BOARD`
- :code:`ANT_NEURO_EE_511_BOARD`

Initialization example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.ANT_NEURO_EE_410_BOARD, params)  # 8 channel amplifier

`More elaborate example <https://github.com/brainflow-dev/brainflow/blob/master/python_package/examples/tests/eego_impedances_and_eeg.py>`_ (reading EEG and impedances)

Supported platforms:

- Windows
- Linux

Available commands:

- Set impedance mode: :code:`board.config_board("impedance_mode:1")`, mode 0 or 1.
- Set sampling rate: :code:`board.config_board("sampling_rate:500")`, for available values check docs from Ant Neuro.
- Set reference range: :code:`board.config_board("reference_range:1.0")`, for available values check docs from Ant Neuro.
- Set bipolar range: :code:`board.config_board("bipolar_range:2.5")`, for available values check docs from Ant Neuro.

For more information about Ant Neuro boards please refer to their User Manual.

Enophone
---------

Enophone Headphones
~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/51374388843_f60c07991e.jpg
    :width: 401px
    :height: 500px

`Enophone website <https://enophone.com/enophones/>`_

**You need to pair your device first.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.ENOPHONE_BOARD`
- :code:`mac_address`, it's optional for some OSes. Windows and MacOS can autodiscover paired devices, Linux cannot

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.mac_address = "F4:0E:11:75:76:78"
    board = BoardShim(BoardIds.ENOPHONE_BOARD, params)

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi

Steps to find MAC address:

- On Windows: open device manager, navigate to enophone device, click properties, details, and select Bluetooth Address
- On Linux: install bluez-utils and run :code:`bluetoothctl paired-devices`
- On MacOS: run :code:`system_profiler SPBluetoothDataType`

On Linux in order to compile and use it you may need to install ``libbluetooth-dev`` for Debian like systems from ``apt-get`` and ``bluez-libs-devel`` for Fedora like systems from ``dnf``.

BrainAlive
-----------

BrainAlive Device
~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/53567093967_39a3cf6c9a.jpg
    :width: 468px
    :height: 500px

`BrainAlive Website <https://brainalive.ai/bci>`_

.. compound::

    On Linux systems you may need to install `libdbus` and we recommend to compile BrainFlow from the source code: ::

        sudo apt-get install libdbus-1-dev # for ubuntu
        sudo yum install dbus-devel # for centos
        python3 tools/build.py --ble # to compile

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.BRAINALIVE_BOARD`
- *optional:* :code:`mac_address`, mac address of the device to connect
- *optional:* :code:`serial_number`, device name

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.BRAINALIVE_BOARD, params)

Supported platforms:

- Windows 10.0.19041.0+
- MacOS 10.15+
- Linux, compilation from source code probably will be needed
- Devices like Raspberry Pi


Mentalab
---------

Explore 4 Channels Board
~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/52349031632_51bc8ea56c.jpg" 
    :width: 500px
    :height: 334px

`Mentalab website <https://mentalab.com/>`_

**You need to pair your device first.**

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.EXPLORE_4_CHAN_BOARD`
- :code:`mac_address`, it's optional for some OSes. Windows and MacOS can autodiscover paired devices, Linux cannot

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.mac_address = "F4:0E:11:75:76:78"
    board = BoardShim(BoardIds.EXPLORE_4_CHAN_BOARD, params)

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi

On Linux in order to compile and use it you may need to install ``libbluetooth-dev`` for Debian like systems from ``apt-get`` and ``bluez-libs-devel`` for Fedora like systems from ``dnf``.

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data
- :code:`BrainFlowPresets.ANCILLARY_PRESET`, it contains battery and temperature data

Steps to find MAC address:

- On Windows: open device manager, navigate to explore device, click properties, details, and select Bluetooth Address
- On Linux: install bluez-utils and run :code:`bluetoothctl paired-devices`
- On MacOS: run :code:`system_profiler SPBluetoothDataType`

Steps to connect:

- Enable device and Pair it with your laptop using bluetooth settings
- Ensure that blue LED is blinking before calling :code:`board.prepare_session()`
- If you see green LED probably you need to reboot a devce

Explore 8 Channels Board
~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/52349031632_51bc8ea56c.jpg" 
    :width: 500px
    :height: 334px

`Mentalab website <https://mentalab.com/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

**You need to pair your enophone device first.**

- :code:`BoardIds.EXPLORE_8_CHAN_BOARD`
- :code:`mac_address`, it's optional for some OSes. Windows and MacOS can autodiscover paired Enophone devices, Linux cannot

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.mac_address = "F4:0E:11:75:76:78"
    board = BoardShim(BoardIds.EXPLORE_8_CHAN_BOARD, params)

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi

On Linux in order to compile and use it you may need to install ``libbluetooth-dev`` for Debian like systems from ``apt-get`` and ``bluez-libs-devel`` for Fedora like systems from ``dnf``.

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG data
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro and Accel data
- :code:`BrainFlowPresets.ANCILLARY_PRESET`, it contains battery and temperature data

Steps to find MAC address:

- On Windows: open device manager, navigate to explore device, click properties, details, and select Bluetooth Address
- On Linux: install bluez-utils and run :code:`bluetoothctl paired-devices`
- On MacOS: run :code:`system_profiler SPBluetoothDataType`

Steps to connect:

- Enable device and Pair it with your laptop using bluetooth settings
- Ensure that blue LED is blinking before calling :code:`board.prepare_session()`
- If you see green LED probably you need to reboot a devce

EmotiBit
---------

EmotiBit board
~~~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/52519313192_7869efa2f5.jpg
    :width: 500px
    :height: 281px

`EmotiBit Website <https://www.emotibit.com/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.EMOTIBIT_BOARD`
- *optional:* :code:`ip_address`, you can provide *broadcast* ip address of the network with EmotiBit device, e.g. 192.168.178.255. If not provided BrainFlow will try to autodiscover the network and it may take a little longer.
- *optional:* :code:`serial_number`, recommended you if have multiple boards in the same network.

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    board = BoardShim(BoardIds.EMOTIBIT_BOARD, params)

Supported platforms:

- Windows
- MacOS
- Linux
- Devices like Raspberry Pi

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains accelerometer, gyroscope and magnetometer data
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains PPG data
- :code:`BrainFlowPresets.ANCILLARY_PRESET`, it contains EDA and temperature data

PiEEG
------

PiEEG Board
~~~~~~~~~~~

PiEEG (Measure EEG with RaspberryPi) – Brain-computer interface (EEG, EMG, and ECG bio-signals) is an open-source Raspberry Pi shield that measures biosignals such as those used in electroencephalography (EEG), electromyography (EMG), and electrocardiography (ECG). It integrates seamlessly with BrainFlow's API, allowing for easy data streaming, processing, and analysis.

.. image:: https://live.staticflickr.com/65535/53823500137_3bf2e27dbf_z.jpg
    :width: 640px
    :height: 384px

`PiEEG Website <https://pieeg.com/>`_

To create such a board, you need to specify the following board ID and fields of the BrainFlowInputParams object:

- :code:`BoardIds.PIEEG_BOARD`
- :code:`serial_port`(optional), e.g. /dev/spidev0.0, etc. More likely you dont need to change the default

Initialization Example:

.. code-block:: python
    from brainflow.board_shim import BoardShim, BrainFlowInputParams, BoardIds
    params = BrainFlowInputParams()
    params.serial_port = "/dev/spidev0.0"
    board = BoardShim(BoardIds.PIEEG_BOARD, params)
    board.prepare_session()
    board.start_stream()

Supported platforms:

- Raspberry Pi

**Note**: Ensure that you have the necessary permissions to access the serial port on your operating system. For Unix-like systems, you may need to configure permissions for the serial port or run with sudo.

**To use this board you need to compile BrainFlow from the source code right on your Raspbery Pi device with flag --build-periphery(build.py) or with -DBUILD_PERIPHERY=ON(CMake) and install desired bindings using local libraries.**

NeuroPawn
--------

Knight Board
~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/54061606098_e223ab04a6_w.jpg
    :width: 400px
    :height: 274px

`NeuroPawn website <https://www.neuropawn.tech/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.NEUROPAWN_KNIGHT_BOARD`
- :code:`serial_port`, e.g. COM3, /dev/tty.*

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.serial_port = "COM3"
    board = BoardShim(BoardIds.NEUROPAWN_KNIGHT_BOARD, params)

**On Unix-like systems you may need to configure permissions for serial port or run with sudo.**

**On MacOS there are two serial ports for each device: /dev/tty..... and /dev/cu..... You HAVE to specify /dev/cu.....**

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi


BioListener
--------

BioListener
~~~~~~~~~~~~~

.. image:: https://live.staticflickr.com/65535/54273076343_6a7eb99697_k.jpg
    :width: 519px
    :height: 389px

`BioListener website <https://github.com/serhii-matsyshyn/biolistener/>`_

To create such board you need to specify the following board ID and fields of BrainFlowInputParams object:

- :code:`BoardIds.BIOLISTENER_BOARD`
- *optional:* :code:`ip_address`, ip address of the machine running the BrainFlow server (not the end device). If not provided, the server will listen on all network interfaces (at `0.0.0.0`)
- *optional:* :code:`ip_port`, any free local port. If the chosen port is in use, the next available free port will be used. If not provided, the search for a free port starts at `12345`
- *optional:* :code:`timeout`, timeout for device discovery, default is 3sec

Make sure to configure the BioListener board as stated in the `BioListener documentation <https://github.com/serhii-matsyshyn/biolistener/>`_ to connect to the BrainFlow server.

Initialization Example:

.. code-block:: python

    params = BrainFlowInputParams()
    params.ip_port = 12345
    params.ip_address = "0.0.0.0"
    board = BoardShim(BoardIds.BIOLISTENER_BOARD, params)

Supported platforms:

- Windows
- Linux
- MacOS
- Devices like Raspberry Pi
- Android

Available :ref:`presets-label`:

- :code:`BrainFlowPresets.DEFAULT_PRESET`, it contains EEG (EMG, ECG, EOG) data
- :code:`BrainFlowPresets.AUXILIARY_PRESET`, it contains Gyro, Accel, battery and ESP32 chip temperature data
