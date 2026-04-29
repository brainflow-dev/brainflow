|PyPI Licence|

SimpleAIBLE
===========

An AI-friendly BLE toolkit powered by `SimpleBLE`_. Scan, connect, and interact with Bluetooth Low Energy devices from AI agents and scripts.

Key Features
------------

* **MCP Server**: Expose BLE operations as tools for MCP-capable clients (Cursor, Claude Code, Windsurf, etc.)
* **HTTP Server**: Control BLE devices over a REST API 
* **Agent Skills**: Teach your AI assistant how to work with Bluetooth devices using reusable skill files
* **Cross-Platform**: Works on Windows, macOS, and Linux

Support & Resources
--------------------

We're here to help you succeed with SimpleAIBLE:

* **Documentation**: Visit our `docs`_ page for comprehensive guides
* **Commercial Support**: Check out |website|_ or |email|_ about licensing and professional services.
* **Community**: Join our `Discord`_ server for discussions and help

**Don't hesitate to reach out if you need assistance - we're happy to help!**

Installation
------------

Install SimpleAIBLE using your preferred package manager:

Using uv (recommended): ::

   uv tool install simpleaible

Or using pip: ::

   pip install simpleaible

MCP Server
----------

Expose BLE operations as tools for MCP-capable clients (Cursor, Claude Code, etc.).
Configure it in your MCP client with the following command: ``"command": "simpleaible-mcp"``.

See the `MCP Server docs`_ for full tool documentation and client-specific setup.

HTTP Server
-----------

Run the REST API for controlling BLE devices remotely: ::

   simpleaible-http --host 127.0.0.1 --port 8000

See the `HTTP Server docs`_ for the full API reference.

Agent Skills
------------

Install the SimpleAIBLE skill to give your AI agent built-in knowledge of BLE workflows: ::

   npx skills add https://github.com/simpleble/simpleble --skill simpleaible

See the `Agent Skills docs`_ for more details.

License
=======

SimpleAIBLE is available under the Business Source License 1.1 (BUSL-1.1). Each
version of SimpleAIBLE will convert to the GNU General Public License version 3 after four years of its initial release.

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

**SimpleAIBLE** is a project powered by |caos|_.

.. Links

.. |email| replace:: email us
.. _email: mailto:contact@simpleble.org

.. |leavemessage| replace:: leave us a message on our website
.. _leavemessage: https://www.simpleble.org/contact?utm_source=pypi_simpleaible&utm_medium=referral&utm_campaign=simpleaible_readme

.. |website| replace:: our website
.. _website: https://simpleble.org?utm_source=pypi_simpleaible&utm_medium=referral&utm_campaign=simpleaible_readme

.. |website-url| replace:: www.simpleble.org
.. _website-url: https://simpleble.org?utm_source=pypi_simpleaible&utm_medium=referral&utm_campaign=simpleaible_readme

.. |caos| replace:: **The California Open Source Company**
.. _caos: https://californiaopensource.com?utm_source=pypi_simpleaible&utm_medium=referral&utm_campaign=simpleaible_readme

.. _SimpleBLE: https://github.com/simpleble/simpleble/

.. _docs: https://docs.simpleble.org/

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _MCP Server docs: https://docs.simpleble.org/simpleaible/mcp

.. _HTTP Server docs: https://docs.simpleble.org/simpleaible/http

.. _Agent Skills docs: https://docs.simpleble.org/simpleaible/skills

.. |PyPI Licence| image:: https://img.shields.io/pypi/l/simpleaible
