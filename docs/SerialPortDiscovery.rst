Serial port discovery
=====================

Scope and API
-------------

This is the first implementation of `issue #187 <https://github.com/brainflow-dev/brainflow/issues/187>`_:
native USB serial enumeration for Windows, Linux and macOS, with automatic selection for
``CYTON_BOARD`` and ``CYTON_DAISY_BOARD``. Other serial boards still require an explicit port.

For these two board IDs:

* A nonempty ``BrainFlowInputParams.serial_port`` always takes precedence and bypasses discovery,
  including when ``serial_number`` is also set. Existing OS port paths and optional libftdi URLs work
  as before.
* An empty ``serial_port`` enumerates USB serial metadata without opening ports, then filters for
  Cyton's FT231X dongle (VID ``0403``, PID ``6015``).
* A nonempty ``serial_number`` additionally requires an exact match to the OS-reported **USB dongle**
  serial number. This is not the Cyton radio channel or a serial number from its firmware.
* Exactly one matching port is required. Zero matches, ambiguity or enumeration failure return
  ``INVALID_ARGUMENTS_ERROR`` before opening any port. Ambiguous candidates and their serial numbers
  are logged; choose an explicit port or a unique dongle serial. Missing or duplicated serial numbers
  cannot resolve ambiguity. An already busy candidate is not silently skipped.
* Only the selected port is opened, once through the normal OS serial path. Its normal handshake must
  succeed. If the device disappears, the open/handshake error is returned without trying another port.

The FT231X IDs describe a USB adapter also used by unrelated products. They do not establish that a
Cyton is attached or powered on. Use an explicit port or known USB serial number on machines with
other FT231X equipment. Clones, reprogrammed IDs and adapters without an OS serial driver require an
explicit port. This implementation does not scan radio channels or identify Cyton versus Daisy.

No C ABI, input JSON fields, exit-code values, board-description schema or language-binding signatures
are added. ``serial_number`` gains the above meaning only for the two supported board IDs when the
port is empty. There is no public candidate-list API in this first increment. The resolved port stays
local to preparation, preserving the caller's session key and allowing a fresh search after failure.

Example with two dongles (replace the serials with the values reported for your devices)::

    a = BrainFlowInputParams()
    a.serial_number = "DONGLE_A"
    b = BrainFlowInputParams()
    b.serial_number = "DONGLE_B"
    first = BoardShim(BoardIds.CYTON_BOARD, a)
    second = BoardShim(BoardIds.CYTON_BOARD, b)
    first.prepare_session()
    second.prepare_session()

Discovery never chooses by enumeration index: COM numbers, USB paths and enumeration order can change
after reconnection. Distinct USB serials or explicit ports are needed for repeatable multi-board use.
There is no new cross-session device reservation mechanism; normal session and OS-open checks apply.

Architecture decision
---------------------

``Serial::get_port_info`` is an internal C++ utility, returning port path, VID, PID and optional USB
serial number. It returns a sorted, deduplicated snapshot and distinguishes an empty list from a
failed/unsupported enumeration. It neither opens serial handles nor sends commands.

.. list-table:: Enumeration alternatives
   :header-rows: 1
   :widths: 22 35 43

   * - Option
     - Advantages
     - Costs / decision
   * - Native metadata backends (chosen)
     - Fits the existing C++11 serial abstraction; no extra redistributable dependency
     - Three small platform backends need CI and hardware verification
   * - libserialport
     - Existing enumeration and USB metadata support on all three desktop platforms
     - Another library to build/package in every distributed binary; LGPL3+ distribution requirements;
       duplicates serial I/O already provided by OSSerial and optional LibFTDISerial
   * - Per-binding enumeration, e.g. pyserial
     - Convenient for Python-only applications
     - Adds a binding-specific dependency and leaves other languages inconsistent
   * - Open each port and probe it
     - Can work without USB metadata
     - Changes unrelated device state, resets adapters and creates races; rejected

The `upstream libserialport README <https://github.com/sigrokproject/libserialport/blob/master/README>`_
documents its supported platforms and license. This implementation does not use its code.

.. list-table:: Native backends
   :header-rows: 1
   :widths: 20 45 35

   * - Platform
     - Implementation
     - Build/runtime requirements
   * - Windows
     - SetupAPI present Ports devices, registry PortName, Config Manager USB-parent traversal;
       only unique physical USB instance IDs supply a serial number
     - System setupapi, cfgmgr32 and advapi32 libraries; installed VCP driver
   * - Linux
     - /sys/class/tty device ancestry and USB idVendor/idProduct/serial attributes; existing /dev nodes
     - libc/sysfs only, no libudev; serial permissions are checked at normal open
   * - macOS
     - IOKit IOSerialBSDClient callout path and physical USB-parent properties
     - System IOKit/CoreFoundation frameworks; only /dev/cu.* aliases, avoiding tty/cu duplicates
   * - Other targets, including Android/iOS
     - Explicit port path remains available; automatic enumeration reports unsupported
     - No desktop IOKit dependency is introduced into iOS builds

USB identity belongs to the transport policy in ``openbci_serial_helpers.cpp``, outside the publicly
returned board-description JSON in ``brainflow_boards.cpp``. One C++ implementation serves all
bindings. Promoting transport descriptors into public board metadata now would imply more stable and
complete device identities than a shared FTDI VID/PID provides. A future public enumeration API can
be designed separately if applications need structured candidate lists.

Further boards need independently verified descriptors and selection semantics. In particular,
Ganglion has a separate dynamically loaded BLED112 path and its ``serial_number`` already selects the
BLE device. FreeEEG has multiple variants and needs verified USB identities. Do not apply the Cyton
USB-serial interpretation globally or infer descriptors from board names. Historical dongle examples
are recorded in the `OpenBCI support discussion <https://openbci.com/forum/index.php?p=/discussion/2821/macos-catalina-10-15-6-usbserial-not-created-resolved>`_.

Startup deadline and buffer policy
----------------------------------

The selected port is configured as before. Input is flushed before sending ``v``, never while waiting
for its welcome. A flush failure is logged, preserving compatibility with older optional libftdi
versions without a flush implementation.

Welcome parsing uses ``steady_clock`` and accepts three consecutive ``$`` bytes, including fragmented
reads. Empty reads do not reset the deadline and include a short backoff. Continuous garbage cannot
extend the deadline; diagnostic storage is capped at 4096 bytes, independently of parsing.
Explicit negative read errors stop immediately (OSSerial currently maps OS read errors to empty reads).

``timeout=0`` uses a six-second welcome deadline; values 1 through 600 specify seconds. Other values
return ``INVALID_ARGUMENTS_ERROR``. This gives the previously unused Cyton timeout field a defined
meaning, including for explicit ports. The deadline controls the welcome, not the entire preparation.
A pending synchronous read may overrun it by one configured read timeout (about 1--1.1 seconds with the
current OS backend); responses arriving after the deadline are rejected.

Verification
------------

Run the dependency-free tests with::

    cmake -S src/tests/serial -B build/serial-tests -DCMAKE_BUILD_TYPE=Release
    cmake --build build/serial-tests --config Release
    ctest --test-dir build/serial-tests -C Release --output-on-failure --timeout 30

They cover zero/one/two matches, reversed order, unrelated adapters, missing IDs, duplicate paths,
missing/duplicate USB serials, exact serial filtering, Windows USB instance parsing, fragmented and
delayed welcome messages, read errors, deadline expiry and bounded diagnostics. Linux fixtures cover
USB ancestry, disappearing ports and malformed attributes; a FIFO port proves enumeration does not
open the device. CI compiles and runs these tests on Windows, Linux and macOS.

The POSIX integration test exercises actual ``prepare_session``/``release_session`` with Cyton and
Daisy pseudo terminals, explicit-port precedence, invalid arguments, failed-handshake retry and two
prepared sessions. Run it against a locally built library, for example::

    python3 src/tests/serial/openbci_serial_pty.py compiled/libBoardController.so

Hardware acceptance checks, required separately from the automated tests:

1. On each desktop OS, verify the returned VID/PID and USB serial against system tools for one real
   dongle, then prepare/start/stop/release with an empty port.
2. Attach two dongles: no filter must fail without opening either; distinct serials must select the
   intended devices. Reverse connection order and repeat. Duplicate/absent serials must require paths.
3. Keep one dongle busy, unplug one between enumeration and open, and test insufficient permissions:
   there must be no automatic fallback to another device.
4. Verify macOS callout-only enumeration and Windows FTDI parent serials with both available VCP
   driver variants; test Linux ttyUSB and USB CDC ancestry.
5. With a powered-off Cyton or delayed welcome, preparation must fail within the configured welcome
   deadline plus one read, and a subsequent retry must release/reopen resources correctly.

Automated tests cannot establish USB-driver metadata correctness or radio/hardware behaviour. This
increment deliberately leaves #187 open for other serial-board families and physical-device validation.
