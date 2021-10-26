### NAME

SPI wrapper functions for Linux userspace `spidev` devices.

### SYNOPSIS

``` c
#include <periphery/spi.h>

/* Primary Functions */
spi_t *spi_new(void);
int spi_open(spi_t *spi, const char *path, unsigned int mode, uint32_t max_speed);
int spi_open_advanced(spi_t *spi, const char *path, unsigned int mode, uint32_t max_speed,
                      spi_bit_order_t bit_order, uint8_t bits_per_word, uint8_t extra_flags);
int spi_open_advanced2(spi_t *spi, const char *path, unsigned int mode, uint32_t max_speed,
                       spi_bit_order_t bit_order, uint8_t bits_per_word, uint32_t extra_flags);
int spi_transfer(spi_t *spi, const uint8_t *txbuf, uint8_t *rxbuf, size_t len);
int spi_close(spi_t *spi);
void spi_free(spi_t *spi);

/* Getters */
int spi_get_mode(spi_t *spi, unsigned int *mode);
int spi_get_max_speed(spi_t *spi, uint32_t *max_speed);
int spi_get_bit_order(spi_t *spi, spi_bit_order_t *bit_order);
int spi_get_bits_per_word(spi_t *spi, uint8_t *bits_per_word);
int spi_get_extra_flags(spi_t *spi, uint8_t *extra_flags);
int spi_get_extra_flags32(spi_t *spi, uint32_t *extra_flags);

/* Setters */
int spi_set_mode(spi_t *spi, unsigned int mode);
int spi_set_max_speed(spi_t *spi, uint32_t max_speed);
int spi_set_bit_order(spi_t *spi, spi_bit_order_t bit_order);
int spi_set_bits_per_word(spi_t *spi, uint8_t bits_per_word);
int spi_set_extra_flags(spi_t *spi, uint8_t extra_flags);
int spi_set_extra_flags32(spi_t *spi, uint32_t extra_flags);

/* Miscellaneous */
int spi_fd(spi_t *spi);
int spi_tostring(spi_t *spi, char *str, size_t len);

/* Error Handling */
int spi_errno(spi_t *spi);
const char *spi_errmsg(spi_t *spi);
```

### ENUMERATIONS

* `spi_bit_order_t`
    * `MSB_FIRST`: Most significant bit first transfer (typical)
    * `LSB_FIRST`: Least significant bit first transfer

### DESCRIPTION

``` c
spi_t *spi_new(void);
```
Allocate a SPI handle.

Returns a valid handle on success, or NULL on failure.

------

``` c
int spi_open(spi_t *spi, const char *path, unsigned int mode, uint32_t max_speed);
```
Open the `spidev` device at the specified path (e.g. "/dev/spidev1.0"), with the specified SPI mode, specified max speed in hertz, and the defaults of `MSB_FIRST` bit order, and 8 bits per word.

`spi` should be a valid pointer to an allocated SPI handle structure. SPI mode can be 0, 1, 2, or 3.

Returns 0 on success, or a negative [SPI error code](#return-value) on failure.

------

``` c
int spi_open_advanced(spi_t *spi, const char *path, unsigned int mode, uint32_t max_speed,
                      spi_bit_order_t bit_order, uint8_t bits_per_word, uint8_t extra_flags);
```
Open the `spidev` device at the specified path, with the specified SPI mode, max speed in hertz, bit order, bits per word, and extra flags.

`spi` should be a valid pointer to an allocated SPI handle structure. SPI mode can be 0, 1, 2, or 3. Bit order can be `MSB_FIRST` or `LSB_FIRST`, as defined [above](#enumerations). Bits per word specifies the transfer word size. Extra flags specified additional flags bitwise-ORed with the SPI mode.

Returns 0 on success, or a negative [SPI error code](#return-value) on failure.

------

``` c
int spi_open_advanced2(spi_t *spi, const char *path, unsigned int mode, uint32_t max_speed,
                       spi_bit_order_t bit_order, uint8_t bits_per_word, uint32_t extra_flags);
```
Open the `spidev` device at the specified path, with the specified SPI mode, max speed in hertz, bit order, bits per word, and extra flags. This open function is the same as `spi_open_advanced()`, except that `extra_flags` can be 32-bits.

`spi` should be a valid pointer to an allocated SPI handle structure. SPI mode can be 0, 1, 2, or 3. Bit order can be `MSB_FIRST` or `LSB_FIRST`, as defined [above](#enumerations). Bits per word specifies the transfer word size. Extra flags specified additional flags bitwise-ORed with the SPI mode.

Returns 0 on success, or a negative [SPI error code](#return-value) on failure.

------

``` c
int spi_transfer(spi_t *spi, const uint8_t *txbuf, uint8_t *rxbuf, size_t len);
```
Shift out `len` word counts of the `txbuf` buffer, while shifting in `len` word counts to the `rxbuf` buffer.

`spi` should be a valid pointer to an SPI handle opened with `spi_open()` or `spi_open_advanced()`.

`rxbuf` may be NULL. `txbuf` and `rxbuf` may point to the same buffer.

Returns 0 on success, or a negative [SPI error code](#return-value) on failure.

------

``` c
int spi_close(spi_t *spi);
```
Close the `spidev` device.

`spi` should be a valid pointer to an SPI handle opened with `spi_open()` or `spi_open_advanced()`.

Returns 0 on success, or a negative [SPI error code](#return-value) on failure.

------

``` c
void spi_free(spi_t *spi);
```
Free a SPI handle.

------

``` c
int spi_get_mode(spi_t *spi, unsigned int *mode);
int spi_get_max_speed(spi_t *spi, uint32_t *max_speed);
int spi_get_bit_order(spi_t *spi, spi_bit_order_t *bit_order);
int spi_get_bits_per_word(spi_t *spi, uint8_t *bits_per_word);
int spi_get_extra_flags(spi_t *spi, uint8_t *extra_flags);
int spi_get_extra_flags32(spi_t *spi, uint32_t *extra_flags);
```
Get the mode, max speed, bit order, bits per word, or extra flags, respectively, of the underlying `spidev` device.

`spi` should be a valid pointer to a SPI handle opened with `spi_open()` or `spi_open_advanced()`.

Returns 0 on success, or a negative [SPI error code](#return-value) on failure.

------

``` c
int spi_set_mode(spi_t *spi, unsigned int mode);
int spi_set_max_speed(spi_t *spi, uint32_t max_speed);
int spi_set_bit_order(spi_t *spi, spi_bit_order_t bit_order);
int spi_set_bits_per_word(spi_t *spi, uint8_t bits_per_word);
int spi_set_extra_flags(spi_t *spi, uint8_t extra_flags);
int spi_set_extra_flags32(spi_t *spi, uint32_t extra_flags);
```
Set the mode, max speed, bit order, bits per word, or extra flags, respectively, on the underlying `spidev` device.

`spi` should be a valid pointer to a SPI handle opened with `spi_open()` or `spi_open_advanced()`.

Returns 0 on success, or a negative [SPI error code](#return-value) on failure.

------

``` c
int spi_fd(spi_t *spi);
```
Return the file descriptor (for the underlying `spidev` device) of the SPI handle.

`spi` should be a valid pointer to a SPI handle opened with `spi_open()` or `spi_open_advanced()`.

This function is a simple accessor to the SPI handle structure and always succeeds.

------

``` c
int spi_tostring(spi_t *spi, char *str, size_t len);
```
Return a string representation of the SPI handle.

`spi` should be a valid pointer to a SPI handle opened with `spi_open()` or `spi_open_advanced()`.

This function behaves and returns like `snprintf()`.

------

``` c
int spi_errno(spi_t *spi);
```
Return the libc errno of the last failure that occurred.

`spi` should be a valid pointer to a SPI handle opened with `spi_open()` or `spi_open_advanced()`.

------

``` c
const char *spi_errmsg(spi_t *spi);
```
Return a human readable error message of the last failure that occurred.

`spi` should be a valid pointer to a SPI handle opened with `spi_open()` or `spi_open_advanced()`.

### RETURN VALUE

The periphery SPI functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `spi_errno()` helper function. A human readable error message can be obtained with the `spi_errmsg()` helper function.

| Error Code            | Description                       |
|-----------------------|-----------------------------------|
| `SPI_ERROR_ARG`       | Invalid arguments                 |
| `SPI_ERROR_OPEN`      | Opening SPI device                |
| `SPI_ERROR_QUERY`     | Querying SPI device attributes    |
| `SPI_ERROR_CONFIGURE` | Configuring SPI device attributes |
| `SPI_ERROR_TRANSFER`  | SPI transfer                      |
| `SPI_ERROR_CLOSE`     | Closing SPI device                |

### EXAMPLE

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

