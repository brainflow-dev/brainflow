Overview
--------

The SimpleBLE project aims to provide fully cross-platform BLE libraries and bindings
for Python and C++, designed for simplicity and ease of use with a licencing scheme
chosen to be friendly towards commercial use. All specific operating system quirks
are handled internally to provide a consistent behavior across all platforms. The
libraries also provide first-class support for vendorization of all third-party
dependencies, allowing for easy integration into existing projects.

This repository offers the source code for the following related libraries:

* **SimpleBLE:** C++ cross-platform BLE library.
* **SimplePyBLE:** Python bindings for SimpleBLE. See `SimplePyBLE`_ PyPI page for more details.
* **SimpleBluez:** C++ abstraction layer for BlueZ over DBus. (Linux only)
* **SimpleDBus:** C++ wrapper for libdbus-1 with convenience classes to handle DBus object hierarchies effectively. (Linux only)

If you want to use SimpleBLE and need help. **Please do not hesitate to reach out!**

* Visit our `ReadTheDocs`_ page.
* Join our `Discord`_ server.
* Contact me: ``kevin at dewald dot me`` (Dedicated consulting services available)

Are you using SimpleBLE on your own project and would like to see it featured here?
Reach out and I'll add a link to it below!

Supported platforms
-------------------
=========== ============= =================================== =====
Windows     Linux         MacOS                               iOS
=========== ============= =================================== =====
Windows 10+ Ubuntu 20.04+ 10.15+ (except 12.0, 12.1 and 12.2) 15.0+
=========== ============= =================================== =====

Projects using SimpleBLE
------------------------
Don't forget to check out the following projects using SimpleBLE:

* `GDSimpleBLE`_
* `BrainFlow`_

Contributing
------------
Pull requests are welcome. For major changes, please open an issue first to discuss
what you would like to change.

License
-------

All components within this project that have not been bundled from
external creators, are licensed under the terms of the `MIT Licence`_.

.. Links

.. _SimplePyBLE: https://pypi.org/project/simplepyble/

.. _MIT Licence: https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/LICENCE.md

.. _fmtlib: https://github.com/fmtlib/fmt

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _ReadTheDocs: https://simpleble.readthedocs.io/en/latest/

.. Other projects using SimpleBLE

.. _GDSimpleBLE: https://github.com/jferdelyi/GDSimpleBLE
.. _BrainFlow: https://github.com/brainflow-dev/brainflow