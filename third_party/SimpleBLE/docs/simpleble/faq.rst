===
FAQ
===

Right now, the best way to have your questions answered is on our `Discord`_ server.
More information will be made available here soon.

**What platforms are supported by SimpleBLE?**
SimpleBLE supports Windows 10+, Linux (Ubuntu 20.04+ and other distros using Bluez),
MacOS 10.15+ (Catalina and newer), iOS 15.0+, and Android API 31+ (Alpha).

Some platform-specific limitations to note:
- Windows and MacOS only support a single Bluetooth adapter
- WSL does not support Bluetooth
- MacOS versions 12.0-12.2 have scanning issues
- Android support is currently in Alpha stage
- iOS support for versions below 15.0 is untested

**Why do I get UUIDs for the peripheral address on MacOS?**

MacOS has a policy of not allowing applications to access the Bluetooth address of
peripherals. Instead, it provides a UUID that is unique to the peripheral. This
UUID is not persistent across reboots, so you should not use it to identify a
peripheral. Instead, you should use the name of the peripheral, which is
persistent across reboots.

**I get a "Bluetooth not enabled" warning on Windows, despite Bluetooth being enabled.**

This is a known issue when running a version of SimpleBLE built for a 32-bit architecture
on a 64-bit Windows machine. The issue is that the underlying Windows API will not allow
us to query the state of the Bluetooth adapter when running in 32-bit mode. The solution
is to consume a 64-bit version of SimpleBLE instead.

**What is the purpose behind the plain interface?**

Building SimpleBLE with the plain-flavored interface allows you to use a version of the
library that is independent of the operating system, which is achieved by hardcoding
all behaviors and responses. This is useful for testing and debugging, especially when
you don't need to deal with operating-system shenanigans and just want to test the API.

**What is the correct approach to memory management of SimpleBLE objects in the C-API?**

**How are vectors supposed to be handled in the C-API?**

.. _Discord: https://discord.gg/N9HqNEcvP3
