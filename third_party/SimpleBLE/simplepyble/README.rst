|PyPI Licence|

SimplePyBLE
===========

The ultimate cross-platform library and bindings for Bluetooth Low Energy (BLE), designed for simplicity and ease of use.

Key Features
------------

* **Cross-Platform**: Enterprise-grade support for Windows, macOS, Linux
* **Easy Integration**: Clean, consistent API across all platforms
* **Multiple Language Bindings**: Production-ready bindings for C, C++, Python, Java and Rust, with more coming soon
* **Commercial Ready**: Source-available commercial license for proprietary applications

Support & Resources
--------------------

We're here to help you succeed with SimpleBLE:

* **Documentation**: Visit our `Documentation`_ page for comprehensive guides
* **Commercial Support**: Check out |website|_ or |email|_ about licensing and professional services.
* **Community**: Join our `Discord`_ server for discussions and help

**Don't hesitate to reach out if you need assistance - we're happy to help!**

Installation
------------

You can install SimplePyBLE from PyPI using pip: ::

   pip install simplepyble

Usage
-----

Please review our `code examples`_ on GitHub for more information on how to use
SimplePyBLE.

Asynchronous Support
--------------------

SimplePyBLE provides an asynchronous API via the ``simplepyble.aio`` module. This module
is designed to work with ``asyncio`` and provides a more idiomatic way to handle
asynchronous operations in Python.

Example: ::

   import asyncio
   from simplepyble.aio import Adapter

   async def main():
       adapters = Adapter.get_adapters()
       adapter = adapters[0]

       async with adapter:
           await adapter.scan_for(5000)
           peripherals = adapter.scan_get_results()
           for peripheral in peripherals:
               print(f"Found: {peripheral.identifier()} [{peripheral.address()}]")

   if __name__ == "__main__":
       asyncio.run(main())

Check out the `async examples`_ for more details.

To run the built-in REST server, you can use the following command: ::

   python3 -m simplepyble.server --host 127.0.0.1 --port 8000


License
=======

Since January 20th 2025, SimpleBLE is now available under the Business Source License 1.1 (BUSL-1.1). Each
version of SimpleBLE will convert to the GNU General Public License version 3 after four years of its initial release.

The project is free to use for non-commercial purposes, but requires a commercial license for commercial use. We
also offer FREE commercial licenses for small projects and early-stage companies - reach out to discuss your use case!

**Why purchase a commercial license?**

- Build and deploy unlimited commercial applications
- Use across your entire development team
- Zero revenue sharing or royalty payments
- Choose features that match your needs and budget
- Priority technical support included
- Clear terms for integrating into MIT-licensed projects

**Looking for information on pricing and commercial terms of service?** Visit |website-url|_ for more details.

For further enquiries, please |email|_ or |leavemessage|_ and we can discuss the specifics of your situation.

----

**SimpleBLE** is a project powered by |caos|_.

.. Links

.. |email| replace:: email us
.. _email: mailto:contact@simpleble.org

.. |leavemessage| replace:: leave us a message on our website
.. _leavemessage: https://www.simpleble.org/contact?utm_source=pypi_simplepyble&utm_medium=referral&utm_campaign=simplepyble_readme

.. |website| replace:: our website
.. _website: https://simpleble.org?utm_source=pypi_simplepyble&utm_medium=referral&utm_campaign=simplepyble_readme

.. |website-url| replace:: www.simpleble.org
.. _website-url: https://simpleble.org?utm_source=pypi_simplepyble&utm_medium=referral&utm_campaign=simplepyble_readme

.. |caos| replace:: **The California Open Source Company**
.. _caos: https://californiaopensource.com?utm_source=pypi_simplepyble&utm_medium=referral&utm_campaign=simplepyble_readme

.. _SimplePyBLE: https://pypi.org/project/simplepyble/

.. _SimpleBLE: https://github.com/simpleble/simpleble/

.. _code examples: https://github.com/simpleble/simpleble/tree/main/examples/simplepyble

.. _async examples: https://github.com/simpleble/simpleble/tree/main/examples/simplepyble

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _Documentation: https://docs.simpleble.org

.. |PyPI Licence| image:: https://img.shields.io/pypi/l/simplepyble
