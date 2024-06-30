### NAME

Serial wrapper functions for Linux userspace termios `tty` devices.

### SYNOPSIS

``` c
#include <periphery/serial.h>

/* Primary Functions */
serial_t *serial_new(void);
int serial_open(serial_t *serial, const char *path, uint32_t baudrate);
int serial_open_advanced(serial_t *serial, const char *path, uint32_t baudrate,
                         unsigned int databits, serial_parity_t parity,
                         unsigned int stopbits, bool xonxoff, bool rtscts);
int serial_read(serial_t *serial, uint8_t *buf, size_t len, int timeout_ms);
int serial_write(serial_t *serial, const uint8_t *buf, size_t len);
int serial_flush(serial_t *serial);
int serial_input_waiting(serial_t *serial, unsigned int *count);
int serial_output_waiting(serial_t *serial, unsigned int *count);
int serial_poll(serial_t *serial, int timeout_ms);
int serial_close(serial_t *serial);
void serial_free(serial_t *serial);

/* Getters */
int serial_get_baudrate(serial_t *serial, uint32_t *baudrate);
int serial_get_databits(serial_t *serial, unsigned int *databits);
int serial_get_parity(serial_t *serial, serial_parity_t *parity);
int serial_get_stopbits(serial_t *serial, unsigned int *stopbits);
int serial_get_xonxoff(serial_t *serial, bool *xonxoff);
int serial_get_rtscts(serial_t *serial, bool *rtscts);

/* Setters */
int serial_set_baudrate(serial_t *serial, uint32_t baudrate);
int serial_set_databits(serial_t *serial, unsigned int databits);
int serial_set_parity(serial_t *serial, enum serial_parity parity);
int serial_set_stopbits(serial_t *serial, unsigned int stopbits);
int serial_set_xonxoff(serial_t *serial, bool enabled);
int serial_set_rtscts(serial_t *serial, bool enabled);

/* Miscellaneous */
int serial_fd(serial_t *serial);
int serial_tostring(serial_t *serial, char *str, size_t len);

/* Error Handling */
int serial_errno(serial_t *serial);
const char *serial_errmsg(serial_t *serial);
```

### ENUMERATIONS

* `serial_parity_t`
    * `PARITY_NONE`: No parity
    * `PARITY_ODD`: Odd parity
    * `PARITY_EVEN`: Even parity

### DESCRIPTION

``` c
serial_t *serial_new(void);
```
Allocate a Serial handle.

Returns a valid handle on success, or NULL on failure.

------

``` c
int serial_open(serial_t *serial, const char *path, uint32_t baudrate);
```
Open the `tty` device at the specified path (e.g. "/dev/ttyUSB0"), with the specified baudrate, and the defaults of 8 data bits, no parity, 1 stop bit, software flow control (xonxoff) off, hardware flow control (rtscts) off.

`serial` should be a valid pointer to an allocated Serial handle structure.

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_open_advanced(serial_t *serial, const char *path, uint32_t baudrate,
                         unsigned int databits, serial_parity_t parity,
                         unsigned int stopbits, bool xonxoff, bool rtscts);
```
Open the `tty` device at the specified path (e.g. "/dev/ttyUSB0"), with the specified baudrate, data bits, parity, stop bits, software flow control (xonxoff), and hardware flow control (rtscts) settings.

`serial` should be a valid pointer to an allocated Serial handle structure. `databits` can be 5, 6, 7, or 8. `parity` can be `PARITY_NONE`, `PARITY_ODD`, or `PARITY_EVEN` as defined [above](#enumerations). `stopbits` can be 1 or 2.

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_read(serial_t *serial, uint8_t *buf, size_t len, int timeout_ms);
```
Read up to `len` number of bytes from the serial port into the `buf` buffer with the specified millisecond timeout. `timeout_ms` can be positive for a blocking read with a timeout in milliseconds, zero for a non-blocking read, or negative for a blocking read that will block until `length` number of bytes are read.

For a non-blocking or timeout-bound read, `serial_read()` may return less than the requested number of bytes.

For a blocking read with the VMIN setting configured, `serial_read()` will block until at least VMIN bytes are read. For a blocking read with both VMIN and VTIME settings configured, `serial_read()` will block until at least VMIN bytes are read or the VTIME interbyte timeout expires after the last byte read. In either case, `serial_read()` may return less than the requested number of bytes.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. `timeout_ms` can be positive for a blocking read with a timeout in milliseconds, zero for a non-blocking read, or negative for a blocking read.

Returns the number of bytes read on success, 0 on timeout, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_write(serial_t *serial, const uint8_t *buf, size_t len);
```
Write `len` number of bytes from the `buf` buffer to the serial port.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

Returns the number of bytes written on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_flush(serial_t *serial);
```
Flush the write buffer of the serial port (i.e. force its write immediately).

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_input_waiting(serial_t *serial, unsigned int *count);
```
Get the number of bytes waiting to be read from the serial port.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_output_waiting(serial_t *serial, unsigned int *count);
```
Get the number of bytes waiting to be written to the serial port.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
bool serial_poll(serial_t *serial, int timeout_ms);
```
Poll for data available for reading from the serial port.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. `timeout_ms` can be positive for a timeout in milliseconds, zero for a non-blocking poll, or negative for a blocking poll.

Returns 1 on success (data available for reading), 0 on timeout, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_close(serial_t *serial);
```
Close the `tty` device.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
void serial_free(serial_t *serial);
```
Free a Serial handle.

------

``` c
int serial_get_baudrate(serial_t *serial, uint32_t *baudrate);
int serial_get_databits(serial_t *serial, unsigned int *databits);
int serial_get_parity(serial_t *serial, serial_parity_t *parity);
int serial_get_stopbits(serial_t *serial, unsigned int *stopbits);
int serial_get_xonxoff(serial_t *serial, bool *xonxoff);
int serial_get_rtscts(serial_t *serial, bool *rtscts);
```
Get the baudrate, data bits, parity, stop bits, software flow control (xonxoff), or hardware flow control (rtscts), respectively, of the underlying `tty` device.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_set_baudrate(serial_t *serial, uint32_t baudrate);
int serial_set_databits(serial_t *serial, unsigned int databits);
int serial_set_parity(serial_t *serial, enum serial_parity parity);
int serial_set_stopbits(serial_t *serial, unsigned int stopbits);
int serial_set_xonxoff(serial_t *serial, bool enabled);
int serial_set_rtscts(serial_t *serial, bool enabled);
```
Set the baudrate, data bits, parity, stop bits, software flow control (xonxoff), or hardware flow control (rtscts), respectively, on the underlying `tty` device.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

Returns 0 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_get_vmin(serial_t *serial, unsigned int *vmin);
int serial_get_vtime(serial_t *serial, float *vtime);
int serial_set_vmin(serial_t *serial, unsigned int vmin);
int serial_set_vtime(serial_t *serial, float vtime);
```
Get or set the termios VMIN and VTIME settings, respectively, of the underlying `tty` device.

VMIN specifies the minimum number of bytes returned from a blocking read. VTIME specifies the timeout in seconds of a blocking read.

When both VMIN and VTIME settings are configured, VTIME acts as an interbyte timeout that restarts on every byte received, and a blocking read will block until either VMIN bytes are read or the VTIME timeout expires after the last byte read. See the `termios` man page for more information.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. `vmin` can be between 0 and 255. `vtime` can be between 0 and 25.5 seconds, with a resolution of 0.1 seconds.

Returns 1 on success, or a negative [Serial error code](#return-value) on failure.

------

``` c
int serial_fd(serial_t *serial);
```
Return the file descriptor (for the underlying `tty` device) of the Serial handle.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

This function is a simple accessor to the Serial handle structure and always succeeds.

------

``` c
int serial_tostring(serial_t *serial, char *str, size_t len);
```
Return a string representation of the Serial handle.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

This function behaves and returns like `snprintf()`.

------

``` c
int serial_errno(serial_t *serial);
```
Return the libc errno of the last failure that occurred.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

------

``` c
const char *serial_errmsg(serial_t *serial);
```
Return a human readable error message of the last failure that occurred.

`serial` should be a valid pointer to a Serial handle opened with `serial_open()` or `serial_open_advanced()`. 

### RETURN VALUE

The periphery Serial functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `serial_errno()` helper function. A human readable error message can be obtained with the `serial_errmsg()` helper function.

| Error Code                | Description                           |
|---------------------------|---------------------------------------|
| `SERIAL_ERROR_ARG`        | Invalid arguments                     |
| `SERIAL_ERROR_OPEN`       | Opening serial port                   |
| `SERIAL_ERROR_QUERY`      | Querying serial port attributes       |
| `SERIAL_ERROR_CONFIGURE`  | Configuring serial port attributes    |
| `SERIAL_ERROR_IO`         | Reading/writing serial port           |
| `SERIAL_ERROR_CLOSE`      | Closing serial port                   |

### EXAMPLE

``` c
#include <stdio.h>
#include <stdlib.h>

#include "serial.h"

int main(void) {
    serial_t *serial;
    const char *s = "Hello World!";
    char buf[128];
    int ret;

    serial = serial_new();

    /* Open /dev/ttyUSB0 with baudrate 115200, and defaults of 8N1, no flow control */
    if (serial_open(serial, "/dev/ttyUSB0", 115200) < 0) {
        fprintf(stderr, "serial_open(): %s\n", serial_errmsg(serial));
        exit(1);
    }

    /* Write to the serial port */
    if (serial_write(serial, s, strlen(s)) < 0) {
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

