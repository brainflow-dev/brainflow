Overview
--------

The SimpleBLE project aims to provide fully cross-platform BLE libraries and bindings
for C++, Python, Rust and other languages, designed for simplicity and ease of use.
All specific operating system quirks are handled internally to provide a consistent behavior
and API across all platforms. The libraries also provide first-class support for vendorization
of all third-party dependencies, allowing for easy integration into existing projects.

**NOTICE: Since February 20, 2024 the license terms of SimpleBLE have changed. Please make sure to read and understand the new licensing scheme.**

This repository offers the source code for the following related libraries:

* **SimpleBLE:** C++ cross-platform BLE library.
* **SimplePyBLE:** Python bindings for SimpleBLE. See `SimplePyBLE`_ PyPI page for more details.
* **SimpleBluez:** C++ abstraction layer for BlueZ over DBus. (Linux only)
* **SimpleDBus:** C++ wrapper for libdbus-1 with convenience classes to handle DBus object hierarchies effectively. (Linux only)

If you want to use SimpleBLE and need help. **Please do not hesitate to reach out!**

* Visit our `ReadTheDocs`_ page.
* Join our `Discord`_ server.
* Contact us: ``contact at simpleble dot org``
* Visit our `website`_ for more information.

Supported platforms
^^^^^^^^^^^^^^^^^^^

Windows
"""""""
* **Supported Versions:** Windows 10 and newer
* **Notes:**

  - WSL does not support Bluetooth.
  - Only a single adapter is supported by the OS backend.

Linux
"""""
* **Supported Distributions:** Ubuntu 20.04 and newer
* **Notes:**

  - While Ubuntu is our primary supported distribution, the software may work on other major distributions using Bluez as their Bluetooth backend.

MacOS
"""""
* **Supported Versions:** 10.15 (Catalina) and newer
* **Exceptions:** MacOS 12.0, 12.1, and 12.2 have a bug where the adapter won't return any peripherals after scanning.
* **Notes:**

  - Only a single adapter is supported by the OS backend.

iOS
"""
* **Supported Versions:** iOS 15.0 and newer
* **Notes:**

  - Older versions of iOS might work but haven't been formally tested.

Android
"""""""
* **Supported Versions:** API 31 and newer
* **Notes:**

  - Capabilities are still in an alpha stage, but should be good enough for initial testing.
  - Older APIs are missing certain features of the JVM API that are required by SimpleBLE

.. Links

.. _website: https://simpleble.org
.. _SimplePyBLE: https://pypi.org/project/simplepyble/
.. _SimpleRsBLE: https://crates.io/crates/simplersble
.. _Discord: https://discord.gg/N9HqNEcvP3
.. _ReadTheDocs: https://simpleble.readthedocs.io/en/latest/

.. Other projects using SimpleBLE

.. _GDSimpleBLE: https://github.com/jferdelyi/GDSimpleBLE
.. _BrainFlow: https://github.com/brainflow-dev/brainflow
.. _InsideBlue: https://github.com/eriklins/InsideBlue-BLE-Tool
.. _NodeWebBluetooth: https://github.com/thegecko/webbluetooth
