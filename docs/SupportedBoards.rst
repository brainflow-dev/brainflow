Supported Boards
=================


Synthetic Board
---------------

BrainFlow board Id is -1

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

BrainFlow Board Id is 0

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

BrainFlow Board Id is 1

Ganglion geting started guide from `OpenBCI 
<https://docs.openbci.com/Tutorials/00-Tutorials>`_.

System Requirements:

- Windows >= 10/Linux/MacOS
- For Python and C# we support x86/x64 libraries, for other bindings x64 only
- If you use python bindings installed from PYPI on Windows you may need to install `redist_x64 <https://aka.ms/vs/16/release/vc_redist.x64.exe>`_ or `redist_x86 <https://aka.ms/vs/16/release/vc_redist.x86.exe>`_  (but more likely you have it preinstalled)

Board Spec:

- num eeg channels: 4
- num acceleration channels: 3
- sampling rate: 200
- communication: Bluetooth Low Energy


OpenBCI Cyton Daisy
-------------------

.. image:: https://live.staticflickr.com/65535/48288597712_7ba142797e.jpg

BrainFlow Board Id is 2

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
