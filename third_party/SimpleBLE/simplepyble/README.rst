SimplePyBLE
===========

The ultimate fully-fledged cross-platform bindings for Bluetooth Low Energy (BLE).

|Latest Documentation Status| |PyPI Licence|

Overview
--------

SimplePyBLE provides Python bindings for `SimpleBLE`_, a fully cross-platform BLE
library written in C++. It is designed for simplicity and ease of use with a
licencing scheme chosen to be friendly towards commercial use.

If you want to use SimpleBLE and need help. **Please do not hesitate to reach out!**

* Visit our `ReadTheDocs`_ page.
* Join our `Discord`_ server.
* Contact me: ``kevin at dewald dot me``

Are you using SimplePyBLE on your own project and would like to see it featured here?
Reach out and I'll add a link to it below!

Supported platforms
-------------------
=========== ============= =================================== =====
Windows     Linux         MacOS                               iOS
=========== ============= =================================== =====
Windows 10+ Ubuntu 20.04+ 10.15+ (except 12.0, 12.1 and 12.2) 15.0+
=========== ============= =================================== =====

**NOTE:** WSL does not support Bluetooth.

Installation
------------

You can install SimplePyBLE from PyPI using pip: ::

   pip install simplepyble

Please review our `code examples`_ on GitHub for more information on how to use
SimplePyBLE.

Contributing
------------
Pull requests are welcome. For major changes, please open an issue first to discuss
what you would like to change.

License
=======

Since February 15th 2024, SimpleBLE is now available under the GNU General Public License
version 3 (GPLv3), with the option for a commercial license without the GPLv3 restrictions
available for a fee.

**More information on pricing and commercial terms of service will be available soon.**

To enquire about a commercial license, please contact us at ``contact at simpleble dot org``.

Likewise, if you are using SimpleBLE in an open-source project and would like to request
a free commercial license or if you have any other questions, please reach out at ``contact at simpleble dot org``.

.. Links

.. _SimplePyBLE: https://pypi.org/project/simplepyble/

.. _SimpleBLE: https://github.com/OpenBluetoothToolbox/SimpleBLE/

.. _code examples: https://github.com/OpenBluetoothToolbox/SimpleBLE/tree/main/examples/simplepyble

.. _Discord: https://discord.gg/N9HqNEcvP3

.. _ReadTheDocs: https://simpleble.readthedocs.io/en/latest/

.. |Latest Documentation Status| image:: https://readthedocs.org/projects/simpleble/badge?version=latest
   :target: http://simpleble.readthedocs.org/en/latest

.. |PyPI Licence| image:: https://img.shields.io/pypi/l/simplepyble

.. Other projects using SimpleBLE

.. _BrainFlow: https://github.com/brainflow-dev/brainflow
.. _InsideBlue: https://github.com/eriklins/InsideBlue-BLE-Tool
.. _NodeWebBluetooth: https://github.com/thegecko/webbluetooth
