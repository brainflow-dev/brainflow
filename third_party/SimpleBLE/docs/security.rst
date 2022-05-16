Security
========

One key security feature of the library is it allows the user to specify
the URLs and tags of all internal dependencies, thus allowing compilation
from internal or secure sources without the risk of those getting compromised.

Vendorization
-------------

Currently, the following libraries are included as part of SimpleBLE, with 
the following CMake options available:

* `fmtlib`_

  * ``LIBFMT_VENDORIZE``: Enable vendorization of fmtlib. *(Default: True)*

  * ``LIBFMT_GIT_REPOSITORY``: The git repository to use for fmtlib.

  * ``LIBFMT_GIT_TAG``: The git tag to use for fmtlib. *(Default: v8.1.1)*

  * ``LIBFMT_LOCAL_PATH``: The local path to use for fmtlib. *(Default: None)*

* `SimpleBluez`_

  * ``SIMPLEBLUEZ_VENDORIZE``: Enable vendorization of SimpleBluez. *(Default: True)*

  * ``SIMPLEBLUEZ_GIT_REPOSITORY``: The git repository to use for SimpleBluez.

  * ``SIMPLEBLUEZ_GIT_TAG``: The git tag to use for SimpleBluez. *(Default: 0.5.0)*

  * ``SIMPLEBLUEZ_LOCAL_PATH``: The local path to use for SimpleBluez. *(Default: None)*

In addition, the following CMake options are available to control vendorization within
SimpleBluez:

* `SimpleDBus`_

  * ``SIMPLEDBUS_VENDORIZE``: Enable vendorization of SimpleDBus. *(Default: True)*

  * ``SIMPLEDBUS_GIT_REPOSITORY``: The git repository to use for SimpleDBus.

  * ``SIMPLEDBUS_GIT_TAG``: The git tag to use for SimpleDBus.

  * ``SIMPLEDBUS_LOCAL_PATH``: The local path to use for SimpleDBus.

:Note: Due to the fact that the development of SimpleBLE is tightly coupled with the
       development of SimpleBluez and SimpleDBus, it is highly recommended that these 
       libraries are consumed in vendorized form, to ensure no unexpected behaviors
       are introduced.

.. Links

.. _fmtlib: https://github.com/fmtlib/fmt

.. _SimpleDBus: https://github.com/OpenBluetoothToolbox/SimpleDBus

.. _SimpleBluez: https://github.com/OpenBluetoothToolbox/SimpleBluez
