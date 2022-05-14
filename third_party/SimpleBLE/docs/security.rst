Security
~~~~~~~~

One key security feature of the library is it allows the user to specify
the URLs and tags of all internal dependencies, thus allowing
compilation from internal or secure sources without the risk of those
getting compromised. This is done by specifying the additional command
line arguments to the ``cmake`` command:

-  ``SIMPLEBLUEZ_GIT_REPOSITORY`` Call CMake with
   ``-DSIMPLEBLUEZ_GIT_REPOSITORY=<path>`` to override the default
   location of the SimpleBluez repository.

-  ``SIMPLEBLUEZ_GIT_TAG`` Call CMake with
   ``-DSIMPLEBLUEZ_GIT_TAG=<tag>`` to override the default tag of the
   SimpleBluez repository.

-  ``SIMPLEDBUS_GIT_REPOSITORY`` Call CMake with
   ``-DSIMPLEDBUS_GIT_REPOSITORY=<path>`` to override the default
   location of the SimpleDBus repository.

-  ``SIMPLEDBUS_GIT_TAG`` Call CMake with ``-DSIMPLEDBUS_GIT_TAG=<tag>``
   to override the default tag of the SimpleDBus repository.

Alternatively, instead of specifying the repository and tag, the user
can specify the path to a local version of the dependencies. This is
done by specifying the additional command line arguments to the
``cmake`` command:

-  ``SIMPLEBLUEZ_LOCAL_PATH`` Call CMake with
   ``-DSIMPLEBLUEZ_LOCAL_PATH=<path>`` to override the default location
   of the SimpleBluez repository.
-  ``SIMPLEDBUS_LOCAL_PATH`` Call CMake with
   ``-DSIMPLEDBUS_LOCAL_PATH=<path>`` to override the default location
   of the SimpleDBus repository.