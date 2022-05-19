SimpleBluez
===========

A simple C++ wrapper around Bluez with a commercially-friendly licence.

|Latest Documentation Status|

Overview
--------

SimpleBluez is a C++ implementation of the `Bluez`_ API on top of `SimpleDBus`_,
conceived to be easy to integrate with a licence friendly for use with
commercial projects.

It's designed to provide complete access to the underlying Bluez stack,
requiring the user to properly handle any associated behaviors and side-effects.

If you want to use SimpleBluez and need help. **Please do not hesitate to reach out!**

* Visit our `ReadTheDocs`_ page. 
* Join our `Discord`_ server.
* Contact me: ``kevin at dewald dot me``

Vendorization
-------------
If you are interested in using SimpleBluez as a dependency in your project,
there is currently first-class support for vendorization of all 3rd-party
dependencies. Currently, the following libraries are included as part of
SimpleBluez:

* `fmtlib`_
* `SimpleDBus`_

Please visit our **Security** page for more information on how to vendorize
3rd-party libraries for SimpleBluez.

License
-------

All components within this project that have not been bundled from
external creators, are licensed under the terms of the `MIT Licence`_.

.. Links

.. _MIT Licence: LICENCE.md

.. _Bluez: https://git.kernel.org/pub/scm/bluetooth/bluez.git/plain/doc/

.. _fmtlib: https://github.com/fmtlib/fmt

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _ReadTheDocs: https://simplebluez.readthedocs.io/en/latest/

.. _SimpleBluez: https://github.com/OpenBluetoothToolbox/SimpleBluez

.. _SimpleDBus: https://github.com/OpenBluetoothToolbox/SimpleDBus

.. |Latest Documentation Status| image:: https://readthedocs.org/projects/simplebluez/badge?version=latest
   :target: http://simplebluez.readthedocs.org/en/latest
