Architecture
============

Layers and their responsibilities
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  External layer

   -  ``SimpleBLE::Adapter`` and ``SimpleBLE::Peripheral`` classes.
   -  These objects hold a shared pointer to ``SimpleBLE::AdapterBase``
      and ``SimpleBLE::PeripheralBase`` respectively.

-  C-style wrapper layer

   -  This layer is a C-style wrapper around the safe C++, designed to
      allow integration of SimpleBLE into other languages that have
      support for C bindings.

-  Safe layer

   -  ``SimpleBLE::AdapterSafe`` and ``SimpleBLE::PeripheralSafe``
      classes.
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

Coding & Naming Conventions
~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  This project follows the coding style guidelines specified in the
   ``.clang-format`` file. They are roughly derived from Google's coding
   standard with a few differences, outlined here:

   -  Class function names are expressed in ``snake_case``.
   -  Class protected and private property names must end with an underscore (``_``).
   -  Class protected and private method names must start with an underscore (``_``).