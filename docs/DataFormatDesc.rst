.. _data-format-label:

Data Format Description
=========================

Units of Measure
------------------

For EXG channels BrainFlow returns uV wherever possible. 

For timestamps BrainFlow uses UNIX timestamp, this counter starts at the Unix Epoch on January 1st, 1970 at UTC.
Precision is microsecond, but for some boards timestamps are generated on PC side as soon as packages were received.

You can compare BrainFlow's timestamp with time returned by code like this:

.. code-block:: python

   import time
   print (time.time ())

Generic Format Description
----------------------------

Methods like:

.. code-block:: python

   get_board_data ()
   get_current_board_data (max_num_packages)

**Return 2d double array [num_channels x num_data_points], rows of this array represent different channels like  EEG channels, EMG channels, Accel channels, Timesteps and so on, while columns in this array represent actual packages from a board.**

Exact format for this array is board specific. To keep the API uniform, we have methods like:

.. code-block:: python

   # these methods return an array of rows in this 2d array containing eeg\emg\ecg\accel data
   get_eeg_channels (board_id)
   get_emg_channels (board_id)
   get_ecg_channels (board_id)
   get_accel_channels (board_id)
   # and so on, check docs for full list
   # also we have methods to get sampling rate from board id, get number of timestamp channel and others
   get_sampling_rate (board_id)
   get_timestamp_channel (board_id)
   # and so on

**For some boards like OpenBCI Cyton, OpenBCI Ganglion, etc we cannot separate EMG, EEG, EDA and ECG and in this case we return exactly the same array for all these methods but for some devices EMG and EEG channels will be different.**

**If board has no such data these methods throw an exception with UNSUPPORTED_BOARD_ERROR exit code.**

Using the methods above, you can write completely board agnostic code and switch boards using a single parameter! Even if you have only one board using these methods you can easily switch to dummy BrainFlow boards and it will help you during developemnent and testing.

Getting All Info About Device And Supported Channels
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There is a method :code:`get_board_descr(int board_id)`. You can use it to get all info about specified device and BrainFlow channels for this board.

.. code-block:: python

   from pprint import pprint 

   import brainflow
   from brainflow.board_shim import BoardShim, BoardIds

   board_id = BoardIds.SYNTHETIC_BOARD.value
   pprint(BoardShim.get_board_descr(board_id))

.. code-block:: json

   {"accel_channels": [17, 18, 19],
    "battery_channel": 29,
    "ecg_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
    "eda_channels": [23],
    "eeg_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
    "eeg_names": "Fz,C3,Cz,C4,Pz,PO7,Oz,PO8,F5,F7,F3,F1,F2,F4,F6,F8",
    "emg_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
    "eog_channels": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16],
    "gyro_channels": [20, 21, 22],
    "marker_channel": 31,
    "name": "Synthetic",
    "num_rows": 32,
    "package_num_channel": 0,
    "ppg_channels": [24, 25],
    "resistance_channels": [27, 28],
    "sampling_rate": 250,
    "temperature_channels": [26],
    "timestamp_channel": 30}


Other Channels
----------------

Some boards have pretty unique data types and we do not have dedicated methods for them, for such devices we return data in :code:`get_other_channels()`. Please refer to the source code to get more info about it.

