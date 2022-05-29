Changelog
=========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog`_, and this project adheres to
`Semantic Versioning`_.

[2.1.0] - 2022-03-25
--------------------

**Added**

*  (Message) Enable the creation of response messages to method calls.
*  (Message) Enable the creation of error messages. 
*  (Connection) Provide interface to query the unique name of the connection.
*  (Proxy) Provide interface to directly append a child.

**Changed**

*  Migrated to using safe callbacks from external vendor (kvn::safe_callback).

**Fixed**

* (Interface) Messages are now passed by reference.

.. _200---2021-12-28:

[2.0.0] - 2021-12-28
--------------------

**Added**

*  Testing framework.
*  Exceptions for error detection and handling.
*  Proxy object to generalize path and message handling.
*  Interface object with advanced features to generalize common
   functionality.
*  Generic callback class to generalize callback functionality.

**Changed**

*  Log levels use now a sane naming convention.
*  Default logging level can be set during build time.
*  Holders containing dictionaries can now support any generic key type.
*  Message types are now part of the class and not a separate enum.

**Removed**

*  Legacy implementations of Property and Introspection.

**Fixed**

*  Interface access is now thread-safe.
*  Proxy access is now thread-safe.

.. _Keep a Changelog: https://keepachangelog.com/en/1.0.0/
.. _Semantic Versioning: https://semver.org/spec/v2.0.0.html
