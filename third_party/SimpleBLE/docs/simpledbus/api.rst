=============
API Reference
=============

Examples
========

To learn how to use SimpleDBus, please refer to the `examples`_ provided
in the repository.


Architecture
============

The following notes provide an overview of the architecture of some of
the higher-level classes in the library, as to facilitate their
understanding.

Interface
----------

-  In order to simplify the routing of messages, all interfaces are
   assumed to have properties, thus skipping the need to have a special
   implementation of org.freedesktop.DBus.Properties.
-  All properties are stored in the holder in which they came from. This
   is not the most efficient way of handling properties, but it is the
   one that minimizes the necessary code for children of the Interface
   class.

Proxy
-----

-  Messages for org.freedesktop.DBus.Properties are automatically
   handled by the Proxy class.


.. Links

.. _examples: https://github.com/OpenBluetoothToolbox/SimpleBLE/tree/main/examples/simpledbus