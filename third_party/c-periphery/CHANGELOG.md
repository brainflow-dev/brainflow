* v2.3.1 - 01/05/2021
    * SPI
        * Fix compilation error and unused variable/parameter warnings when
          building under Linux kernel headers without 32-bit SPI mode flags
          support.
    * Contributors
        * Ryan Barnett, @rjbarnet - 708f7fe, 21c1b7a

* v2.3.0 - 12/16/2020
    * MMIO
        * Add advanced open function with device path for use with
          alternate memory character devices (e.g. `/dev/gpiomem`).
    * SPI
        * Add getter and setter for 32-bit extra flags.
        * Add advanced open function with 32-bit extra flags.
    * Build
        * Enable unused parameter warning.
    * Contributors
        * Rémy Dziemiaszko, @remdzi - b8adb42

* v2.2.5 - 11/19/2020
    * GPIO
        * Add direction checks for improved error reporting to `gpio_write()`,
          `gpio_read_event()`, and `gpio_poll()` for character device GPIOs.
        * Improve string handling in `gpio_open()` and in getters for
          sysfs and character device GPIOs.
    * LED
        * Improve string handling in `led_open()` and `led_name()`.
    * Build
        * Add default optimization to CFLAGS in Makefile.
        * Add debug and release CFLAGS to CMakeLists.txt.

* v2.2.4 - 09/11/2020
    * Fix future spurious close caused by uncleared handle state after an error
      during open in GPIO, I2C, SPI, Serial, and MMIO modules.

* v2.2.3 - 09/03/2020
    * GPIO
        * Disable character device GPIO support when building with older Linux
          kernel headers missing line event support in the gpio-cdev ABI.
    * SPI
        * Fix formatted bits per word truncation in `spi_tostring()`.
    * Build
        * Add test for character device GPIO support in Linux kernel headers to
          Makefile.
    * Contributors
        * Fabrice Fontaine, @ffontaine - 5b81b89

* v2.2.2 - 07/24/2020
    * GPIO
        * Add conditional compilation of character device GPIO support to allow
          build under older Linux kernel headers.
        * Increase feature test macro version to fix missing definition
          warnings.
    * Build
        * Fix directory paths for pkg-config pc file generation and
          installation under CMake.
        * Fix COMMIT_ID identification when building within a parent git
          repository under CMake.
        * Add CMake build option for tests.
    * Contributors
        * oficsu, @oficsu - 80bc63d
        * Ryan Barnett, @rjbarnet - ea1e0da, 05262e6, 50fcd0a
        * Fabrice Fontaine, @ffontaine - caadb46

* v2.2.1 - 05/31/2020
    * GPIO
        * Add feature test macro for POLLRDNORM flag to fix build with uClibc.
        * Fix argument name in prototype for `gpio_set_bias()`.
    * Contributors
        * Joris Offouga, @jorisoffouga - cfc722e

* v2.2.0 - 05/29/2020
    * GPIO
        * Add `gpio_poll_multiple()` function.
        * Add getter for line consumer label.
        * Add getters and setters for line bias, line drive, and inverted
          properties.
        * Add advanced open functions with additional properties for character
          device GPIOs.
        * Only unexport GPIO in `gpio_close()` if exported in
          `gpio_open_sysfs()` for sysfs GPIOs.
        * Add retry loop to direction write after export to accommodate delayed
          udev permission rule application in `gpio_open_sysfs()` for sysfs
          GPIOs.
        * Improve wording and fix typos in documentation.
    * Serial
        * Add getters and setters for vmin and vtime termios settings.
        * Add support for termios timeout with `serial_read()`.
        * Improve wording in documentation.
    * Build
        * Add CMake build support.
        * Add pkg-config pc file generation.
    * Contributors
        * Joris Offouga, @jorisoffouga - 952e1e9, 671e618

* v2.1.0 - 01/07/2020
    * Add LED module.
    * Add PWM module.
    * Clean up internal string handling in SPI and GPIO modules.

* v2.0.1 - 10/08/2019
    * Initialize handle state in new functions of all modules.
    * Fix performance of blocking read in `serial_read()`.
    * Return error on unexpected empty read in `serial_read()`, which may be
      caused by a serial port disconnect.
    * Improve formatting of `spi_tostring()`.
    * Fix typo in GPIO module documentation.
    * Fix cross-compilation support in Makefile to allow override of CC
      variable.

* v2.0.0 - 09/30/2019
    * Add support for character device GPIOs (`gpio-cdev`) to the GPIO module.
        * Remove support for preserve direction in `gpio_open()`.
        * Remove problematic dummy read with sysfs GPIOs from `gpio_poll()`.
        * Unexport sysfs GPIOs in `gpio_close()`.
    * Migrate to opaque handles with new/free functions in all modules.
    * Simplify error codes for MMIO, I2C, and Serial modules.
    * Fix typos in GPIO module documentation.
    * Update tests with running hints for Raspberry Pi 3.
    * Improve cross-compilation support in Makefile.
    * Contributors
        * longsky, @wangqiang1588 - d880ef7
        * jhlim, @johlim - 742d983

* v1.1.3 - 04/28/2018
    * Fix data's most significant bit getting stripped when opening a serial
      port with parity enabled in `serial_open_advanced()`.
    * Contributors
        * Ryan Barnett, @rjbarnet - 537eeac

* v1.1.2 - 04/01/2018
    * Add handling for delayed pin directory export on some platforms in
      `gpio_open()`.
    * Fix supported functions query for 64-bit in `i2c_open()`.
    * Add support for building with C++.
    * Contributors
        * Jared Bents, @jmbents - 304faf4
        * Ryan Barnett, @rjbarnet - 82ebb4f

* v1.1.1 - 04/25/2017
    * Fix blocking `gpio_poll()` for some platforms.
    * Add library version macros and functions.
    * Contributors
        * Михаил Корнилов, @iTiky - 0643fe9

* v1.1.0 - 09/27/2016
    * Add support for preserving pin direction to `gpio_open()`.
    * Fix enabling input parity check in `serial_set_parity()`.
    * Fix enabling hardware flow control in `serial_set_rtscts()`.
    * Include missing header to fix build with musl libc.
    * Omit unsupported serial baudrates to fix build on SPARC.
    * Contributors
        * Thomas Petazzoni - 27a9552, 114c715

* v1.0.3 - 05/25/2015
    * Fix portability of serial baud rate set/get with termios-provided baud rate functions.
    * Fix unlikely bug in `spi_tostring()` formatting.
    * Clean up integer argument signedness in serial API.

* v1.0.2 - 01/31/2015
    * Fix `gpio_supports_interrupts()` so it does not return an error if interrupts are not supported.
    * Fix `errno` preservation in a few error paths, mostly in the open functions.

* v1.0.1 - 12/26/2014
    * Improve Makefile.
    * Fix _BSD_SOURCE compilation warnings.

* v1.0.0 - 05/15/2014
    * Initial release.
