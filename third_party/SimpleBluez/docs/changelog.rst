Changelog
=========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog`_, and this project adheres to
`Semantic Versioning`_.

[0.4.0] - 2022-04-07
--------------------

**Added**

*  ``RSSI`` property to ``Device`` class.
*  Adapters can now return a list of all paired devices.

[0.3.1] - 2022-04-02
--------------------

**Changed**

*  By default, all pairing options will succeed.


[0.3.0] - 2022-03-25
--------------------

**Added**

*  ``Agent`` and ``AgentManager`` classes to handle pairing.
*  Pair and Notify examples.

**Changed**

*  Migrated to using safe callbacks from external vendor (kvn::safe_callback).

[0.2.1] - 2022-02-13
--------------------

**Changed**

*  Minor renaming of function for style consistency.

[0.2.0] - 2022-02-12
--------------------

**Added**

*  Support for the ``Battery1`` interface. *(Thanks ptenbrock!)*

[0.1.1] - 2021-12-28
--------------------

**Added**

*  Function to access currently cached value from characteristics.
*  ``Notifying`` property on ``Characteristic1``.
*  Added ``OnDisconnected`` callback to ``Device1``.
*  Added ``ServicesResolved`` callback to ``Device1``.
*  Address and Thread sanitization options.

**Changed**

*  All proxy and interface manipulation is now done through helper functions.
*  Access to all interface properties is now thread-safe.

**Fixed**

*  Removed unnecessary ``<iostream>`` includes.
*  Made sure all classes have proper virtual destructors.

[0.1.0] - 2021-12-14
--------------------

Reimplementation of the library based on the SimpleDBus v2.0.0-alpha.2 API.

[0.0.1] - 2021-11-09
--------------------

First working implementation post migration, following the SimpleDBus v2.0.0-alpha API.

.. _Keep a Changelog: https://keepachangelog.com/en/1.0.0/
.. _Semantic Versioning: https://semver.org/spec/v2.0.0.html
