SimpleBLE
==========

The ultimate fully-fledged cross-platform library and bindings for Bluetooth Low Energy (BLE).

|Latest Documentation Status|

Overview
--------

The SimpleBLE project aims to provide fully cross-platform BLE libraries and bindings
for C++, Python, Rust and other languages, designed for simplicity and ease of use.
All specific operating system quirks are handled internally to provide a consistent behavior
and API across all platforms. The libraries also provide first-class support for vendorization
of all third-party dependencies, allowing for easy integration into existing projects.

**NOTICE: Since February 20, 2024 the license terms of SimpleBLE have changed. Please make sure to read and understand the details below.**

Below you'll find a list of components that are part of SimpleBLE:

* **SimpleBLE:** C++ cross-platform BLE library.
* **SimplePyBLE:** Python bindings for SimpleBLE. See the `SimplePyBLE`_ PyPI page for more details.
* **SimpleDroidBLE:** Android-specific package following the SimpleBLE API. (Still in Alpha, more to come)
* **SimpleRsBLE:** Rust bindings for SimpleBLE (LEGACY - Big refactor coming soon). See the `SimpleRsBLE`_ Crates.io page for more details.
* **SimpleBluez:** C++ abstraction layer for BlueZ over DBus. (Linux only)
* **SimpleDBus:** C++ wrapper for libdbus-1 with convenience classes to handle DBus object hierarchies effectively. (Linux only)

If you want to use SimpleBLE and need help. **Please do not hesitate to reach out!**

* Visit our `ReadTheDocs`_ page.
* Join our `Discord`_ server.
* Contact us: ``contact at simpleble dot org``
* Visit our `website`_ for more information.

Supported platforms
-------------------

.. list-table::
   :header-rows: 1

   * - Platform
     - Versions
     - Notes
   * - Windows
     - Windows 10+
     - • WSL does not support Bluetooth.
       • Only a single adapter is supported by the OS backend.
   * - Linux
     - Ubuntu 20.04+
     - • Other distros using Bluez as their Bluetooth backend should work.
   * - MacOS
     - 10.15+ (Catalina and newer)
     - • MacOS 12.0, 12.1, and 12.2 have a bug where the adapter won't return any peripherals after scanning.
       • Only a single adapter is supported by the OS backend.
   * - iOS
     - 15.0+
     - • Older versions of iOS might work but haven't been formally tested.
   * - Android (Alpha)
     - API 31+
     - • Older APIs are missing certain features of the JVM API that are required by SimpleBLE.

Projects using SimpleBLE
------------------------
Don't forget to check out the following projects using SimpleBLE:

* `BrainFlow`_
* `InsideBlue`_
* `NodeWebBluetooth`_

Contributing
------------
Pull requests are welcome. For major changes, please open an issue first to discuss
what you would like to change.

License
=======

Since February 15th 2024, SimpleBLE is now available under the GNU General Public License
version 3 (GPLv3), with the option for a commercial license without the GPLv3 restrictions
available for a fee.

**You can find more information on pricing and commercial terms of service on our `website`_.**

For further enquiries, please contact us at ``contact at simpleble dot org``.

Licensing FAQ
-------------

I'm already using SimpleBLE. What happens to my project?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Please reach out at ``contact at simpleble dot org`` and we can discuss the specifics of your
situation. It is my intention to make this transition as smooth as possible for existing users,
and I'm open to finding a solution that works for everyone.

If you are using SimpleBLE in an open-source project and would like to request
a free commercial license or if you have any other questions, do not hesitate to reach out.

Why are you making this change?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

So far, SimpleBLE has been a labor of passion with over 4000 hours invested in multiple iterations.
The decision to transition SimpleBLE to a dual-licensing model is mainly driven by the kind
of products that have been built around it, in particular around notable names in the medical
and industrial sectors, which has been both surprising and encouraging. Providing robust support for
these diverse and critical use cases is a resource-intensive endeavor which can't be achieved on
goodwill alone, especially so when the underlying APIs are also evolving and life having its own
plans. By introducing a commercial license, we're opening a pathway to dedicate more resources to
enhance SimpleBLE. Some of the things on the roadmap include:

- Bindings into more languages and frameworks.
- Hardware-in-the-loop test infrastructure.
- Offering bounties and revenue sharing with other developers who contribute.
- Providing more comprehensive documentation and tutorials.

Despite this transition, We remain firmly committed to the open-source philosophy. SimpleBLE was grown
a lot thanks to the feedback of the open-source community, and that foundation will always be a part
of the project. The GPLv3 license option ensures continued accessibility for open-source projects,
and we pledge to actively contribute to and collaborate with the community whenever possible.

Ultimately, the success of SimpleBLE has been fueled by its open nature, and we believe this
dual-licensing model strengthens that success by enabling both community-driven growth and
targeted enhancements that benefit everyone.

What does the GPLv3 license imply for my commercial project?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The GPLv3 license ensures that end users have the freedom to run, study, share, and modify the software.
It requires that any modified versions of SimpleBLE, or any software incorporating it, also be
distributed under the GPLv3. Essentially, if your project incorporates SimpleBLE and is distributed,
the entire codebase must be open-source under the GPLv3.

You can find the full text of the GPLv3 license at https://www.gnu.org/licenses/gpl-3.0.html.

.. Links

.. _website: https://simpleble.org

.. _SimplePyBLE: https://pypi.org/project/simplepyble/

.. _SimpleRsBLE: https://crates.io/crates/simplersble

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _ReadTheDocs: https://simpleble.readthedocs.io/en/latest/

.. |Latest Documentation Status| image:: https://readthedocs.org/projects/simpleble/badge?version=latest
   :target: http://simpleble.readthedocs.org/en/latest

.. Other projects using SimpleBLE

.. _GDSimpleBLE: https://github.com/jferdelyi/GDSimpleBLE
.. _BrainFlow: https://github.com/brainflow-dev/brainflow
.. _InsideBlue: https://github.com/eriklins/InsideBlue-BLE-Tool
.. _NodeWebBluetooth: https://github.com/thegecko/webbluetooth
