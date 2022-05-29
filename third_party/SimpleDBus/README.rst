SimpleDBus
==========

A simple C++ wrapper around DBus with a commercially-friendly licence.

|Latest Documentation Status|

Overview
--------

SimpleDBus is a C++ low-level binding around libdbus conceived to be easy
to integrate with a licence friendly for use with commercial projects.

This library is designed to be an extremely minimalistic high-level
wrapper around libdbus, with most of the focus placed on proper
packing/unpacking of DBus data structures. It's primary intended
use case is by `SimpleBluez`_, a project to provide easy access to
the Bluez subsystem on Linux.

If you want to use SimpleDBus and need help. **Please do not hesitate to reach out!**

* Visit our `ReadTheDocs`_ page. 
* Join our `Discord`_ server.
* Contact me: ``kevin at dewald dot me``

Vendorization
-------------
If you are interested in using SimpleDBus as a dependency in your project,
there is currently first-class support for vendorization of all 3rd-party
dependencies. Currently, the following libraries are included as part of
SimpleDBus:

* `fmtlib`_

Please visit our **Security** page for more information on how to vendorize
3rd-party libraries for SimpleDBus.

License
-------

All components within this project that have not been bundled from
external creators, are licensed under the terms of the `MIT Licence`_.

.. Links

.. _MIT Licence: LICENCE.md

.. _fmtlib: https://github.com/fmtlib/fmt

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _ReadTheDocs: https://simpledbus.readthedocs.io/en/latest/

.. _SimpleBluez: https://github.com/OpenBluetoothToolbox/SimpleBluez

.. |Latest Documentation Status| image:: https://readthedocs.org/projects/simpledbus/badge?version=latest
   :target: http://simpledbus.readthedocs.org/en/latest
