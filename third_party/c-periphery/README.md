# c-periphery [![Build Status](https://travis-ci.com/vsergeev/c-periphery.svg?branch=master)](https://travis-ci.com/vsergeev/c-periphery) [![GitHub release](https://img.shields.io/github/release/vsergeev/c-periphery.svg?maxAge=7200)](https://github.com/vsergeev/c-periphery) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/vsergeev/c-periphery/blob/master/LICENSE)

## C Library for Linux Peripheral I/O (GPIO, LED, PWM, SPI, I2C, MMIO, Serial)

c-periphery is a small C library for GPIO, LED, PWM, SPI, I2C, MMIO, and Serial peripheral I/O interface access in userspace Linux. c-periphery simplifies and consolidates the native Linux APIs to these interfaces. c-periphery is useful in embedded Linux environments (including Raspberry Pi, BeagleBone, etc. platforms) for interfacing with external peripherals. c-periphery is re-entrant, has no dependencies outside the standard C library and Linux, compiles into a static library for easy integration with other projects, and is MIT licensed.

Using Python or Lua? Check out the [python-periphery](https://github.com/vsergeev/python-periphery) and [lua-periphery](https://github.com/vsergeev/lua-periphery) projects.

Contributed libraries: [java-periphery](https://github.com/sgjava/java-periphery), [dart_periphery](https://github.com/pezi/dart_periphery)

## Examples

### GPIO

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gpio.h"

int main(void) {
    gpio_t *gpio_in, *gpio_out;
    bool value;

    gpio_in = gpio_new();
    gpio_out = gpio_new();

    /* Open GPIO /dev/gpiochip0 line 10 with input direction */
    if (gpio_open(gpio_in, "/dev/gpiochip0", 10, GPIO_DIR_IN) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(gpio_in));
        exit(1);
    }

    /* Open GPIO /dev/gpiochip0 line 12 with output direction */
    if (gpio_open(gpio_out, "/dev/gpiochip0", 12, GPIO_DIR_OUT) < 0) {
        fprintf(stderr, "gpio_open(): %s\n", gpio_errmsg(gpio_out));
        exit(1);
    }

    /* Read input GPIO into value */
    if (gpio_read(gpio_in, &value) < 0) {
        fprintf(stderr, "gpio_read(): %s\n", gpio_errmsg(gpio_in));
        exit(1);
    }

    /* Write output GPIO with !value */
    if (gpio_write(gpio_out, !value) < 0) {
        fprintf(stderr, "gpio_write(): %s\n", gpio_errmsg(gpio_out));
        exit(1);
    }

    gpio_close(gpio_in);
    gpio_close(gpio_out);

    gpio_free(gpio_in);
    gpio_free(gpio_out);

    return 0;
}
```

[Go to GPIO documentation.](docs/gpio.md)

### LED

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "led.h"

int main(void) {
    led_t *led;
    unsigned int max_brightness;

    led = led_new();

    /* Open LED led0 */
    if (led_open(led, "led0") < 0) {
        fprintf(stderr, "led_open(): %s\n", led_errmsg(led));
        exit(1);
    }

    /* Turn on LED (set max brightness) */
    if (led_write(led, true) < 0) {
        fprintf(stderr, "led_write(): %s\n", led_errmsg(led));
        exit(1);
    }

    /* Get max brightness */
    if (led_get_max_brightness(led, &max_brightness) < 0) {
        fprintf(stderr, "led_get_max_brightness(): %s\n", led_errmsg(led));
        exit(1);
    }

    /* Set half brightness */
    if (led_set_brightness(led, max_brightness / 2) < 0) {
        fprintf(stderr, "led_set_brightness(): %s\n", led_errmsg(led));
        exit(1);
    }

    led_close(led);

    led_free(led);

    return 0;
}
```

[Go to LED documentation.](docs/led.md)

### PWM

``` c
#include <stdio.h>
#include <stdlib.h>

#include "pwm.h"

int main(void) {
    pwm_t *pwm;

    pwm = pwm_new();

    /* Open PWM chip 0, channel 10 */
    if (pwm_open(pwm, 0, 10) < 0) {
        fprintf(stderr, "pwm_open(): %s\n", pwm_errmsg(pwm));
        exit(1);
    }

    /* Set frequency to 1 kHz */
    if (pwm_set_frequency(pwm, 1e3) < 0) {
        fprintf(stderr, "pwm_set_frequency(): %s\n", pwm_errmsg(pwm));
        exit(1);
    }

    /* Set duty cycle to 75% */
    if (pwm_set_duty_cycle(pwm, 0.75) < 0) {
        fprintf(stderr, "pwm_set_duty_cycle(): %s\n", pwm_errmsg(pwm));
        exit(1);
    }

    /* Enable PWM */
    if (pwm_enable(pwm) < 0) {
        fprintf(stderr, "pwm_enable(): %s\n", pwm_errmsg(pwm));
        exit(1);
    }

    /* Change duty cycle to 50% */
    if (pwm_set_duty_cycle(pwm, 0.50) < 0) {
        fprintf(stderr, "pwm_set_duty_cycle(): %s\n", pwm_errmsg(pwm));
        exit(1);
    }

    pwm_close(pwm);

    pwm_free(pwm);

    return 0;
}
```

[Go to PWM documentation.](docs/pwm.md)

### SPI

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "spi.h"

int main(void) {
    spi_t *spi;
    uint8_t buf[4] = { 0xaa, 0xbb, 0xcc, 0xdd };

    spi = spi_new();

    /* Open spidev1.0 with mode 0 and max speed 1MHz */
    if (spi_open(spi, "/dev/spidev1.0", 0, 1000000) < 0) {
        fprintf(stderr, "spi_open(): %s\n", spi_errmsg(spi));
        exit(1);
    }

    /* Shift out and in 4 bytes */
    if (spi_transfer(spi, buf, buf, sizeof(buf)) < 0) {
        fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spi));
        exit(1);
    }

    printf("shifted in: 0x%02x 0x%02x 0x%02x 0x%02x\n", buf[0], buf[1], buf[2], buf[3]);

    spi_close(spi);

    spi_free(spi);

    return 0;
}
```

[Go to SPI documentation.](docs/spi.md)

### I2C

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "i2c.h"

#define EEPROM_I2C_ADDR 0x50

int main(void) {
    i2c_t *i2c;

    i2c = i2c_new();

    /* Open the i2c-0 bus */
    if (i2c_open(i2c, "/dev/i2c-0") < 0) {
        fprintf(stderr, "i2c_open(): %s\n", i2c_errmsg(i2c));
        exit(1);
    }

    /* Read byte at address 0x100 of EEPROM */
    uint8_t msg_addr[2] = { 0x01, 0x00 };
    uint8_t msg_data[1] = { 0xff, };
    struct i2c_msg msgs[2] =
        {
            /* Write 16-bit address */
            { .addr = EEPROM_I2C_ADDR, .flags = 0, .len = 2, .buf = msg_addr },
            /* Read 8-bit data */
            { .addr = EEPROM_I2C_ADDR, .flags = I2C_M_RD, .len = 1, .buf = msg_data},
        };

    /* Transfer a transaction with two I2C messages */
    if (i2c_transfer(i2c, msgs, 2) < 0) {
        fprintf(stderr, "i2c_transfer(): %s\n", i2c_errmsg(i2c));
        exit(1);
    }

    printf("0x%02x%02x: %02x\n", msg_addr[0], msg_addr[1], msg_data[0]);

    i2c_close(i2c);

    i2c_free(i2c);

    return 0;
}
```

[Go to I2C documentation.](docs/i2c.md)

### MMIO

``` c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <byteswap.h>

#include "mmio.h"

struct am335x_rtcss_registers {
    uint32_t seconds;       /* 0x00 */
    uint32_t minutes;       /* 0x04 */
    uint32_t hours;         /* 0x08 */
    /* ... */
};

int main(void) {
    mmio_t *mmio;
    uint32_t mac_id0_lo, mac_id0_hi;
    volatile struct am335x_rtcss_registers *regs;

    mmio = mmio_new();

    /* Open Control Module */
    if (mmio_open(mmio, 0x44E10000, 0x1000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

    /* Read lower 2 bytes of MAC address */
    if (mmio_read32(mmio, 0x630, &mac_id0_lo) < 0) {
        fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

    /* Read upper 4 bytes of MAC address */
    if (mmio_read32(mmio, 0x634, &mac_id0_hi) < 0) {
        fprintf(stderr, "mmio_read32(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

    printf("MAC address: %08X%04X\n", __bswap_32(mac_id0_hi), __bswap_16(mac_id0_lo));

    mmio_close(mmio);

    /* Open RTC subsystem */
    if (mmio_open(mmio, 0x44E3E000, 0x1000) < 0) {
        fprintf(stderr, "mmio_open(): %s\n", mmio_errmsg(mmio));
        exit(1);
    }

    regs = mmio_ptr(mmio);

    /* Read current RTC time */
    printf("hours: %02x minutes: %02x seconds %02x\n", regs->hours, regs->minutes, regs->seconds);

    mmio_close(mmio);

    mmio_free(mmio);

    return 0;
}
```

[Go to MMIO documentation.](docs/mmio.md)

### Serial

``` c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serial.h"

int main(void) {
    serial_t *serial;
    uint8_t s[] = "Hello World!";
    uint8_t buf[128];
    int ret;

    serial = serial_new();

    /* Open /dev/ttyUSB0 with baudrate 115200, and defaults of 8N1, no flow control */
    if (serial_open(serial, "/dev/ttyUSB0", 115200) < 0) {
        fprintf(stderr, "serial_open(): %s\n", serial_errmsg(serial));
        exit(1);
    }

    /* Write to the serial port */
    if (serial_write(serial, s, sizeof(s)) < 0) {
        fprintf(stderr, "serial_write(): %s\n", serial_errmsg(serial));
        exit(1);
    }

    /* Read up to buf size or 2000ms timeout */
    if ((ret = serial_read(serial, buf, sizeof(buf), 2000)) < 0) {
        fprintf(stderr, "serial_read(): %s\n", serial_errmsg(serial));
        exit(1);
    }

    printf("read %d bytes: _%s_\n", ret, buf);

    serial_close(serial);

    serial_free(serial);

    return 0;
}
```

[Go to Serial documentation.](docs/serial.md)

## Building c-periphery with CMake

### Static library

Build c-periphery into a static library:

``` console
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Shared Library

Build c-periphery into a shared library:

``` console
$ mkdir build
$ cd build
$ cmake -DBUILD_SHARED_LIBS=ON ..
$ make
```

Install the shared library and headers:

``` console
$ sudo make install
```

### Tests

Build c-periphery tests from the build directory:

``` console
$ make tests
```

### Cross-compilation

Set the `CC` environment variable with the cross-compiler prior to build:

``` console
$ export CC=arm-linux-gnueabihf-gcc
$ mkdir build
$ cd build
$ cmake ..
$ make
```

If additional cross-compiler tools are needed, use a `CMAKE_TOOLCHAIN_FILE` to fully specify the toolchain parameters:

``` console
$ mkdir build
$ cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/arm-linux-gnueabihf.cmake ..
$ make
```

## Building c-periphery with vanilla Make

### Static library

Build c-periphery into a static library:

``` console
$ make
```

### Tests

Build c-periphery tests:

``` console
$ make tests
```

### Cross-compilation

Set the `CROSS_COMPILE` environment variable with the cross-compiler prefix when building:

``` console
$ CROSS_COMPILE=arm-linux-gnueabihf- make
```

## Building c-periphery into another project statically

Include the header files from `src/` and link in the `periphery.a` static library:

``` console
$ gcc -I/path/to/periphery/src myprog.c /path/to/periphery/periphery.a -o myprog
```

## Building c-periphery into another project dynamically

If the header files and shared library are installed on the system, simply link with `-lperiphery`:

``` console
$ gcc myprog.c -lperiphery -o myprog
```

Otherwise, additional include (`-I`) and library (`-L`) paths may be required.

## Documentation

`man` page style documentation for each interface wrapper is available in [docs](docs/) folder.

## Testing

The tests located in the [tests](tests/) folder may be run to test the correctness and functionality of c-periphery. Some tests require interactive probing (e.g. with an oscilloscope), the installation of a physical loopback, or the existence of a particular device on a bus. See the usage of each test for more details on the required test setup.

## License

c-periphery is MIT licensed. See the included [LICENSE](LICENSE) file.

