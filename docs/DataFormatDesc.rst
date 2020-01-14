.. _data-format-label:

Data Format Description
=========================

Generic Format Description
----------------------------

**Methods like:**

.. code-block:: python

   get_board_data ()
   get_current_board_data (max_num_packages)

**Return 2d double array [num_channels x num_data_points], rows of this array represent different channels like  EEG channels, EMG channels, Accel channels, Timesteps and so on, while columns in this array represent actual packages from a board.**

Exact format for this array is board specific. To keep the API uniform. we have methods like:

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

**For some boards like OpenBCI Cyton, OpenBCI Ganglion and others we can not separate EMG, EEG, EDA and ECG and in this case we return exactly the same array for all these methods but for some boards EMG and EEG channels differ**

Using the methods above, you can write completely board agnostic code and switch boards using a single parameter! Even if you have only one board using these methods you can easily switch to Synthetic board for development and run code without real hardware.

Special channels for Cyton Based boards
-----------------------------------------

`Cyton based boards from OpenBCI <https://docs.openbci.com/docs/02Cyton/CytonLanding>`_ suport different output formats which is described `here <https://docs.openbci.com/docs/02Cyton/CytonDataFormat#firmware-version-200-fall-2016-to-now-1>`_.

**For Cyton based boards, we add Cyton End byte to a first channel from:**

.. code-block:: python

   get_other_channels (board_id)

**If Cyton End bytes is equal to 0xC0 we add accel data. To get rows which contain accel data use:**

.. code-block:: python

   get_accel_channels (board_id)

**If Cyton End bytes is equal to 0xC1 we add analog data. To get rows which contain analog data use:**

.. code-block:: python

   get_analog_channels (board_id)

For analog data, we return int32 values. From low level API, we return double array, so these values are cast to double without any changes.

**Also we add raw unprocessed bytes to the second and next channels returned by:**

.. code-block:: python

   get_other_channels (board_id)

If Cyton End Byte is outside `this range <https://docs.openbci.com/docs/02Cyton/CytonDataFormat#firmware-version-200-fall-2016-to-now-1>`_, we drop the entire package.

**Check this example for details:**

.. literalinclude:: ../tests/python/cyton_analog_mode_other_data.py
   :language: py
