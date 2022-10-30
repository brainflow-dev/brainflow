========
Tutorial
========

In this page you'll find all the information you need to get up and running with SimpleBLE.


Getting started
===============

Your first step towards using SimpleBLE is to download and install the library
following the instructions in the `usage <usage.html>`_ page. Once you have
installed the library, you need to understand a few basic concepts about how
SimpleBLE is organized.

To access all classes provided by SimpleBLE, you can go the easy route and
include everything with just one single header file::

   #include <simpleble/SimpleBLE.h>

In SimpleBLE, everything revolves around two main object types: A :cpp:class:`SimpleBLE::Adapter`
representing a physical Bluetooth adapter, and a :cpp:class:`SimpleBLE::Peripheral` representing the
Bluetooth device you are communicating with. If you wish to be more specific and
only consume what you need, you have the following list of headers available.
Take into account that each header file will automatically include the ones
underneath in the list. ::

   #include <simpleble/Adapter.h>
   #include <simpleble/Peripheral.h>
   #include <simpleble/Service.h>
   #include <simpleble/Characteristic.h>
   #include <simpleble/Descriptor.h>

One you have the correct header files included, you have two functions that should
act as a starting point for every program. The first one is :cpp:func:`SimpleBLE::Adapter::bluetooth_enabled()`,
which will let you know if Bluetooth is enabled and permissions have been given
to the running app. The second one is :cpp:func:`SimpleBLE::Adapter::get_adapters()`, which
will provide a list of all available adapters that can be used. The following
code snippet shows how to use these functions::

   #include <simpleble/SimpleBLE.h>

   int main(int argc, char** argv) {
      if (!SimpleBLE::Adapter::bluetooth_enabled()) {
         std::cout << "Bluetooth is not enabled" << std::endl;
         return 1;
      }

      auto adapters = SimpleBLE::Adapter::get_adapters();
      if (adapters.empty()) {
         std::cout << "No Bluetooth adapters found" << std::endl;
         return 1;
      }

      // Use the first adapter
      auto adapter = adapters[0];

      // Do something with the adapter
      std::cout << "Adapter identifier: " << adapter.identifier() << std::endl;
      std::cout << "Adapter address: " << adapter.address() << std::endl;

      return 0;
   }

The above code will print the identifier and address of the first adapter found
using :cpp:func:`SimpleBLE::Adapter::identifier()` and :cpp:func:`SimpleBLE::Adapter::address()`, respectively.
If you have more than one adapter, you can use the identifier to select the
one you want to use. The identifier is a string that uniquely identifies the
adapter within the operating system. The address is a string that represents
the MAC address of the adapter.


Scanning for peripherals
========================

Once you have a list of available adapters, you can start scanning for
peripherals. To do so, you need to create an :cpp:class:`SimpleBLE::Adapter` object
and call the :cpp:func:`SimpleBLE::Adapter::scan_for()` method. This method will
return a list of :cpp:class:`SimpleBLE::Peripheral` objects that are in range
of the adapter. ::

   // Get a list of all available adapters
   std::vector<SimpleBLE::Adapter> adapters = SimpleBLE::Adapter::get_adapters();

   // Get the first adapter
   SimpleBLE::Adapter adapter = adapters[0];

   // Scan for peripherals for 5000 milliseconds
   adapter.scan_for(5000);

   // Get the list of peripherals found
   std::vector<SimpleBLE::Peripheral> peripherals = adapter.scan_get_results();

   // Print the identifier of each peripheral
   for (auto peripheral : peripherals) {
       std::cout << "Peripheral identifier: " << peripheral.identifier() << std::endl;
       std::cout << "Peripheral address: " << peripheral.address() << std::endl;
   }

The above code will print the identifier and address of each peripheral found
using :cpp:func:`SimpleBLE::Peripheral::identifier()` and :cpp:func:`SimpleBLE::Peripheral::address()`, respectively.

Additionally, you can use :cpp:func:`SimpleBLE::Adapter::scan_start()` and
:cpp:func:`SimpleBLE::Adapter::scan_stop()` to start and stop scanning asynchronously.
This is useful if you want to scan for peripherals in the background while
performing other tasks. For this use case you can supply callback functions
to receive notifications for different scan-related events by calling
:cpp:func:`SimpleBLE::Adapter::set_callback_on_scan_start()`, :cpp:func:`SimpleBLE::Adapter::set_callback_on_scan_stop()`,
:cpp:func:`SimpleBLE::Adapter::set_callback_on_scan_updated()` and :cpp:func:`SimpleBLE::Adapter::set_callback_on_scan_found()`.
The following code snippet shows how to use these functions::

   // Set the callback to be called when the scan starts
   adapter.set_callback_on_scan_start([]() {
      std::cout << "Scan started" << std::endl;
   });

   // Set the callback to be called when the scan stops
   adapter.set_callback_on_scan_stop([]() {
      std::cout << "Scan stopped" << std::endl;
   });

   // Set the callback to be called when the scan finds a new peripheral
   adapter.set_callback_on_scan_found([](SimpleBLE::Peripheral peripheral) {
      std::cout << "Peripheral found: " << peripheral.identifier() << std::endl;
   });

   // Set the callback to be called when a peripheral property has changed
   adapter.set_callback_on_scan_updated([](SimpleBLE::Peripheral peripheral) {
      std::cout << "Peripheral updated: " << peripheral.identifier() << std::endl;
   });

   // Start scanning for peripherals
   adapter.scan_start();

   // Wait for 5 seconds
   std::this_thread::sleep_for(std::chrono::seconds(5));

   // Stop scanning for peripherals
   adapter.scan_stop();


Connecting to a peripheral
==========================

Once you have a list of peripherals, you can connect to one of them. To do so,
you need to create a :cpp:class:`SimpleBLE::Peripheral` object and call the
:cpp:func:`SimpleBLE::Peripheral::connect()` method. ::

   // Scan for peripherals for 5000 milliseconds
   std::vector<SimpleBLE::Peripheral> peripherals = adapter.scan_for(5000);

   // Connect to the first peripheral
   SimpleBLE::Peripheral peripheral = peripherals[0];
   peripheral.connect();

.. note::
   **More coming soon, I swear. :P**

Learn by example
================

To learn how to use SimpleBLE, please refer to the `examples`_ provided
in the repository. Those examples named with a ``_c`` suffix are using
the C-wrapped version of the library and those examples with a ``_safe``
suffix use the _noexcept_ version of the library.

The following list briefly describes each example provided:

* `list_adapters`_: List all available adapters.
* `scan (cpp)`_ & `scan (c)`_: Scan for nearby BLE devices.
* `connect (cpp)`_ & `connect_safe (cpp)`_ & `connect (c)`_: Connect to a BLE device and list its services and characteristics.
* `read`_: Read a characteristic's value.
* `write`_: Write a characteristic's value.
* `notify (cpp)`_ & `notify (c)`_: Enable notifications on a characteristic.

Concurrency
===========

When designing your application using SimpleBLE, concurrency is a key
aspect that needs to be taken into account. This is because internally
the library relies on a thread pool to handle asynchronous operations
and poll the operating system's Bluetooth stack, which can suffer from
contention and potentially cause the program to crash or freeze if these
threads are significantly delayed.

This can have an important effect when using SimpleBLE with UI
applications, such as WxWidgets or Unity.

Layers and their responsibilities
=================================

-  External layer

   -  ``SimpleBLE::Adapter`` and ``SimpleBLE::Peripheral`` classes.
   -  These objects hold a shared pointer to ``SimpleBLE::AdapterBase``
      and ``SimpleBLE::PeripheralBase`` respectively.

-  C-style wrapper layer

   -  This layer is a C-style wrapper around the safe C++, designed to
      allow integration of SimpleBLE into other languages that have
      support for C bindings.

-  Safe layer

   -  ``SimpleBLE::AdapterSafe`` and ``SimpleBLE::PeripheralSafe`` classes.
   -  These objects wrap all ``SimpleBLE::Adapter`` and
      ``SimpleBLE::Peripheral`` objects and provide an interface that
      does not throw exceptions. Instead, it will return an
      ``std::optional<T>`` object if the function returns a value, or a
      boolean indicating whether the function succeeded if the original
      function did not return a value. The usage is functionally
      equivalent to their respective counterparts in the external layer.

-  API layer (OS-dependent)

   -  ``SimpleBLE::AdapterBase`` and ``SimpleBLE::PeripheralBase``
      classes.
   -  These classes specify the API of the library on top of which the
      external layer is actually wrapping.
   -  Each OS target has to implement the full public API specified in
      the external layer, using private methods and properties for the
      specific requirements of each environment.
   -  Two convenience classes, ``SimpleBLE::AdapterBuilder`` and
      ``SimpleBLE::PeripheralBuilder`` are provided for the case of
      allowing access to private methods during the build process.

.. Links

.. _examples: https://github.com/OpenBluetoothToolbox/SimpleBLE/tree/main/examples/simpleble

.. _list_adapters: https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/cpp/list_adapters/list_adapters.cpp

.. _scan (cpp): https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/cpp/scan/scan.cpp

.. _scan (c): https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/c/scan/scan.c

.. _connect (cpp): https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/cpp/connect/connect.cpp

.. _connect_safe (cpp): https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/cpp/connect_safe/connect_safe.cpp

.. _connect (c): https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/c/connect/connect.c

.. _read: https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/cpp/read/read.cpp

.. _write: https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/cpp/write/write.cpp

.. _notify (cpp): https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/cpp/notify/notify.cpp

.. _notify (c): https://github.com/OpenBluetoothToolbox/SimpleBLE/blob/main/examples/simpleble/c/notify/notify.c
