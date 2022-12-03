===
FAQ
===

Right now, the best way to have your questions answered is on our `Discord`_ server.
More information will be made available here soon.

**Why do I get UUIDs for the peripheral address on MacOS?**

MacOS has a policy of not allowing applications to access the Bluetooth address of
peripherals. Instead, it provides a UUID that is unique to the peripheral. This
UUID is not persistent across reboots, so you should not use it to identify a
peripheral. Instead, you should use the name of the peripheral, which is
persistent across reboots.

**What is the purpose behind the plain interface?**

Building SimpleBLE with the plain-flavored interface allows you to use a version of the
library that is independent of the operating system, which is achieved by hardcoding
all behaviors and responses. This is useful for testing and debugging, especially when
you don't need to deal with operating-system shenanigans and just want to test the API.

**What is the correct approach to memory management of SimpleBLE objects in the C-API?**

**How are vectors supposed to be handled in the C-API?**

.. _Discord: https://discord.gg/N9HqNEcvP3
