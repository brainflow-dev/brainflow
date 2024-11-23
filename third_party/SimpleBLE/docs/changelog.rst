Changelog
=========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog`_, and this project adheres to `Semantic Versioning`_.

[0.8.2] - XXXX-XX-XX
--------------------

**Added**

-

**Changed**

-

**Fixed**

-


[0.8.1] - 2024-11-05
--------------------

**Added**

- (Android) Alpha preview of Android support.
- (SimpleDBus) Added templated version of creation and getter functions for Holder class. *(Thanks lorsi96!)*

**Changed**

- Implemented standalone ByteArray class derived from `kvn::bytearray`. *(Thanks tlifschitz!)*
-  **API CHANGE**: Notify and Indicate callback in C bindings now receive the peripheral handle as the first argument.

**Fixed**

- (SimpleBluez) Fixed improper handling of non `org.Bluez.Service1` objects within a `org.bluez.Device1` object. *(Thanks Kober Engineering!)*
- (MacOS) Fixed incorrect storage and retrieval with standard Bluetooth UUIDs inside the peripheral class. *(Thanks TellowKrinkle!)*
- (Python) Fixed incorrect handling of the GIL in certain functions. *(Thanks nomenquis and Medra AI!)*


[0.7.X]
--------------------

This entire series is dedicated to reviewing and updating the license terms of the project.


[0.7.0] - 2024-02-15
--------------------

**Added**

- Function to query the version of SimpleBLE at runtime.
- (Python) Missing API from SimpleBLE::Characteristic.

**Changed**

- (MacOS) Main adapter address is now hardcoded to allow caching based on adapter address. *(Thanks BlissChapman!)*
- (Python) Release GIL when calling ``Peripheral.write_request`` and ``Peripheral.write_command``.
- (MacOS) Rewrote the entire backend.
- (MacOS) OperationFailed exception now contains the error message provided by the OS.

**Fixed**

- (MacOS) Remove unnecessary timeout during service discovery. *(Thanks BlissChapman!)*
- (MacOS) Return correct list of devices when scanning. *(Thanks roozmahdavian!)*
- (MacOS) Remove unnecessary timeout during characteristic notification. *(Thanks BlissChapman!)*
- (MacOS) Remove unnecessary timeout during operations on characteristics.
- (Windows) Failed connection attempt would not trigger an exception. *(Thanks eriklins!)*
- (Linux) Use correct UUIDs for advertized services. *(Thanks Symbitic!)*


[0.6.1] - 2023-03-14
--------------------

**Added**

- (Python) Generate source distribution packages.
- (SimpleDBus) Proxy objects keep track of their existence on the DBus object tree.

**Changed**

- Bluetooth enabled check was moved into the frontend modules. *(Thanks felixdollack!)*
- (Windows) Use the standard C++ exception handling model. *(Thanks TheFrankyJoe!)*

**Fixed**

- CI artifacts for non-standard architectures are now properly built.
- (SimpleBluez) Fixed incorrect handling of invalidated children objects.


[0.6.0] - 2023-02-23
--------------------

**Added**

-  Option to build SimpleBLE plain-flavored (without any BLE code) for testing and debugging purposes.
-  Support for advertized services.
-  Support for GATT Characteristic properties.
-  Retrieve the MTU value of an established connection. *(Thanks Marco Cruz!)*
-  Peripheral addresses can now be queried for their type. *(Thanks camm73!)*
-  Tx Power is decoded from advertising data if available. *(Thanks camm73!)*
-  Logger now provides default functions to log to a file or to stdout.
-  Support for exposing advertized service data. *(Thanks Symbitic!)*
-  (Rust) Preliminary implementation of Rust bindings.
-  (Windows) Logging of WinRT initialization behavior.
-  (SimpleBluez) Support for GATT characteristic flags.
-  (SimpleBluez) Support for GATT characteristic MTUs. *(Thanks Marco Cruz!)*
-  (SimpleBluez) Support for advertized services.
-  (SimpleBluez) Mechanism to select the default DBus bus type during compilation-time. *(Thanks MrMinos!)*

**Changed**

-  Debug, MinSizeRel and RelWithDebInfo targets now contain their appropriate suffix. *(Thanks kutij!)*
-  **API CHANGE**: Log level convention changed from uppercase to capitalizing the first letter.
-  Updated ``libfmt`` dependency to version 9.1.0.
-  Unused ``libfmt`` targets removed from the build process.
-  (MacOS) More explicit exception messages.
-  (MacOS) 16-bit UUIDs are now presented in their 128-bit form.
-  (MacOS) Adapter address now swapped for a random UUID. *(Thanks nothingisdead!)*
-  (Windows) Reinitialize the WinRT backend if a single-threaded apartment is detected. *(Thanks jferdelyi!)*
-  (Windows) Callbacks for indications and notifications are now swapped if one already exists.

**Fixed**

-  Incorrect handling of services and characteristics in the Python examples. *(Thanks Carl-CWX!)*
-  Minor potential race condition in the safe callback.
-  Compilation-time log levels were not being set correctly. *(Thanks chen3496!)*
-  Missing function definition in C-bindings. *(Thanks eriklins!)*
-  (Linux) Peripheral would still issue callbacks after deletion.
-  (MacOS) Increased priority of the dispatch queue to prevent jitter in the incoming data.
-  (MacOS) Incorrect listing of advertized services. *(Thanks eriklins & Symbitic!)*
-  (Windows) Missing peripheral identifier data. *(Thanks eriklins!)*
-  (Windows) Multiple initializations of the WinRT backend.
-  (Windows) Incorrect initialization of the WinRT backend. *(Thanks ChatGPT & Andrey1994!)*
-  (Windows) Scan callbacks would continue after scan stopped.
-  (Windows) Disconnecting would prevent the user from connecting again. *(Thanks klaff, felixdollack & lairdrt!)*
-  (Windows) Uncleared callbacks when unsubscribe is called.
-  (Windows) Incorrect handling of non-english locale by MSVC. *(Thanks felixdollack!)*
-  (Windows) Disconnection callback would not be triggered on a manual disconnect. *(Thanks crashtua!)*
-  (Python) Type returned by ``simplepyble.get_operating_system()`` was not defined.
-  (SimpleBluez) Incorrect attempt to operate on an uninitialized DBus connection. *(Thanks jacobbreen25!)*


[0.5.0] - 2022-09-25
--------------------

**Important:**
 -  From this version onwards, the CMake target that should be consumed by downstream projects is ``simpleble::simpleble``.
 -  This version includes a breaking API change in the enumeration of services and characteristics.
 -  This version has brought in the files from SimpleBluez and SimpleDBus into the repository as subpackages.

**Added**

-  Multiple connection example.
-  Installation interface.
-  Logger level and callback can now be queried.
-  Characteristics can now list their descriptors. *(Thanks Symbitic!)*
-  Peripherals can now read and write characteristic descriptors. *(Thanks Symbitic!)*
-  Adapter object can now be queried to see if Bluetooth is enabled.
-  (Windows) WinRT exception handling.
-  (Windows) Accessor function to underlying OS objects of ``Adapter`` and ``Peripheral``.
-  (MacOS) Failures will now throw corresponding exception.
-  (SimpleBluez) Support for characteristic descriptors. *(Thanks Symbitic!)*
-  (SimpleBluez) Full support for all discovery filters. *(Thanks Symbitic!)*

**Changed**

-  Clearer layout of examples. *(Thanks Yohannfra!)*
-  ``AdapterSafe`` and ``PeripheralSafe`` will now catch all exceptions.
-  Selection of build type is now based on the  ``BUILD_SHARED_LIBS`` setting.
-  Consumable CMake target is now ``simpleble::simpleble``.
-  **API CHANGE**: ``BluetoothService`` class was replaced by the ``Service`` class.
-  Updated CMake minimum version to 3.21
-  Symbols are now hidden by default and use proper export mechanics.
-  Logger will print to std::out by default.
-  (MacOS) Stop throwing exceptions if Bluetooth not enabled. Print warning and no-op instead.
-  (Linux) Default scanning behavior switched to all devices.

**Fixed**

-  Made user callback invocations exception-safe.
-  Attempting to scan while connected will erase references to all existing peripherals.
-  CMake target ``simpleble::simpleble`` was removed in favour of ``BUILD_SHARED_LIBS``.
-  CMake target ``simpleble::simpleble-c`` was removed in favour of ``BUILD_SHARED_LIBS``.
-  Using the correct CMake functionality to export headers for all targets.
-  Corrected maximum length of manufacturer data on the C-api to 27 bytes. *(Thanks DrSegatron!)*
-  (Windows) Peripheral reads are now uncached. *(Thanks piotromt!)*
-  (Linux) Failure to set agent would trigger a crash.
-  (Linux) Spurious disconnection events during connection retries have been fully removed.
-  (Linux) Exceptions thrown during the deletion phase of a peripheral would not be captured.
-  (Linux) Characteristic cleanup function has been made exception-safe.
-  (SimpleBluez) Accessing the ``Paired`` property of ``Device1`` would only use the cached value.


[0.4.0] - 2022-06-12
--------------------

**Added**

-  Expose RSSI as a property of ``Peripheral``.
-  Utils function to identify the current platform.
-  (Linux) ``Peripheral::is_paired`` method to check if a peripheral is paired.
-  (Linux) ``Adapter::get_paired_peripherals`` method to list all paired peripherals.
-  Function to validate whether an ``Adapter`` or ``Peripheral`` object is initialized.
-  Logging hooks to capture logs from SimpleBLE and internal components.
-  Accessor function to underlying OS objects of ``Adapter`` and ``Peripheral``.
-  (Python) Python's Global Interpreter Lock (GIL) will be released during ``Peripheral.connect()``.
-  (Python) Keep-alive policies for function objects passed into SimplePyBLE.

**Changed**

-  Updated Linux implementation to use SimpleBluez v0.5.0.
-  Added support for Windows SDK 10.0.22000.0
-  Updated ``libfmt`` to version 8.1.1.
-  Cleaned up dependency management for ``libfmt`` and SimpleBluez.
-  ``Adapter::get_paired_peripherals`` will return an empty list on Windows and MacOS.
-  (Linux) **(Experimental)** Exceptions thrown inside the Bluez async thread are now caught to prevent lockups.
-  ``NotConnected`` exception will be thrown instead of ``OperationFailed`` when peripheral not connected.

**Fixed**

-  (MacOS) Known peripherals would not get cleared at the beginning of a scanning session.
-  (Windows) Known peripherals would not get cleared at the beginning of a scanning session.
-  Calling functions of uninitialized objects will now throw an exception instead of crashing.
-  (MacOS) Thread synchronization issues would cause certain peripheral actions to report failure.
-  (Windows) Behavior of ``write_request`` and ``write_command`` was flipped.
-  (MacOS) Behavior of ``write_request`` and ``write_command`` was flipped.
-  (Linux) ``on_connected`` callback was not being called.
-  (Linux) Spurious disconnection events during connection retries have been removed.
-  (Linux) Existing characteristic callbacks were not being cleared on disconnection.
-  (Linux) Characteristics are unsubscribed on disconnection.
-  (Linux) Missing agent registration that would prevent pairing from working.

[0.3.0] - 2022-04-03
--------------------

**Added**

-  Pairing functionality has been validated on all supported operating systems.
   In the case of Windows and MacOS, the user will be required to interact with
   an operating system popup to pair the device, while on Linux all pairing
   requests will automatically be accepted, with passcodes ``abc123`` or ``123456``.
-  Unpair command has been added, although the only working implementation
   will be the Linux one. Both Windows and MacOS require the user to manually
   unpair a device from the corresponding OS settings page.

**Changed**

-  Updated Linux implementation to use SimpleBluez v0.3.1.
-  Migrated to using safe callbacks from external vendor (kvn::safe_callback).

[0.2.0] - 2022-02-13
--------------------

**Added**

-  (Linux) Support for emulated battery service. *(Thanks ptenbrock!)*

**Fixed**

-  (Windows) Proper cleanup of callbacks during destruction.
-  (Windows) Async timeout reduced to 10 seconds.
-  (Linux) Returned characteristic value would be empty or outdated. *(Thanks ptenbrock!)*
-  (MacOS) Fixed a bunch of memory leaks and enabled automatic reference counting.
-  (MacOS) Fixed race condition.
-  (Python) ``write_request`` and ``write_command`` functions would accept strings instead of bytes as payloads. *(Thanks kaedenbrinkman!)*

**Changed**

-  Updated Linux implementation to use SimpleBluez v0.2.1.


[0.1.0] - 2021-12-28
--------------------

**Changed**

-  Referenced specific version of SimpleBluez to avoid breaking changes as those libraries evolve.
-  (Linux) When ``scan_stop`` is called, it is now guaranteed that no more scan results will be received.
-  Updated Linux implementation to use SimpleBluez v0.1.1.

**Fixed**

-  (Linux) Scan will never stop sleeping.


[0.0.2] - 2021-10-09
--------------------

**Added**

-  Safe implementation of ``Adapter`` and ``Peripheral`` classes.
-  CppCheck and ClangFormat CI checks. *(Thanks Andrey1994!)*
-  C-style API with examples.
-  Access to manufacturer data in the ``Peripheral`` class, for Windows and MacOS.

**Fixed**

-  Compilation errors that came up during development. *(Thanks fidoriel!)*
-  WinRT buffer allocation would fail. *(Thanks PatrykSajdok!)*
-  ``Adapter`` would fail to stop scanning. *(Thanks PatrykSajdok!)*
-  Switched WinRT initialization to single-threaded.

**Changed**

-  SimpleBluez dependency migrated to OpenBluetoothToolbox.


[0.0.1] - 2021-09-06
--------------------

**Added**

-  Initial definition of the full API.
-  Usage examples of the library.

.. _Keep a Changelog: https://keepachangelog.com/en/1.0.0/
.. _Semantic Versioning: https://semver.org/spec/v2.0.0.html
