SimpleBLE
==========

The ultimate fully-fledged cross-platform library for Bluetooth Low Energy (BLE).

|Latest Documentation Status|

Overview
--------

SimpleBLE is a C++ fully cross-platform BLE library designed for simplicity
and ease of use. All specific operating system quirks are handled to provide
a consistent behavior across all platforms. Each major version of the library
will have a stable API that will be fully forwards compatible. 

If you want to use SimpleBLE and need help. **Please do not hesitate to reach out!**

* Visit our `ReadTheDocs`_ page. 
* Join our `Discord`_ server.
* Contact me: ``kevin at dewald dot me``

Are you using SimpleBLE on your own project? Reach out and I'll add a link to it below!

Supported platforms
-------------------
=========== ============= =================================== =====
Windows     Linux         MacOS                               iOS
=========== ============= =================================== =====
Windows 10+ Ubuntu 20.04+ 10.15+ (except 12.0, 12.1 and 12.2) 15.0+
=========== ============= =================================== =====

Vendorization
-------------
If you are interested in using SimpleBLE as a dependency in your project,
there is currently first-class support for vendorization of all 3rd-party
dependencies. Currently, the following libraries are included as part of
SimpleBLE:

* `fmtlib`_
* `SimpleBluez`_
* `SimpleDBus`_

Please visit our **Security** page for more information on how to vendorize
3rd-party libraries for SimpleBLE.

License
-------

All components within this project that have not been bundled from
external creators, are licensed under the terms of the `MIT Licence`_.

.. Links

.. _MIT Licence: LICENCE.md

.. _fmtlib: https://github.com/fmtlib/fmt

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _ReadTheDocs: https://simpleble.readthedocs.io/en/latest/

.. _SimpleBluez: https://github.com/OpenBluetoothToolbox/SimpleBluez

.. _SimpleDBus: https://github.com/OpenBluetoothToolbox/SimpleDBus

.. |Latest Documentation Status| image:: https://readthedocs.org/projects/simpleble/badge?version=latest
   :target: http://simpleble.readthedocs.org/en/latest
