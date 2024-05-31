### NAME

I2C wrapper functions for Linux userspace `i2c-dev` devices.

### SYNOPSIS

``` c
#include <periphery/i2c.h>

/* Primary Functions */
i2c_t *i2c_new(void);
int i2c_open(i2c_t *i2c, const char *device);
int i2c_transfer(i2c_t *i2c, struct i2c_msg *msgs, size_t count);
int i2c_close(i2c_t *i2c);
void i2c_free(i2c_t *i2c);

/* Miscellaneous */
int i2c_fd(i2c_t *i2c);
int i2c_tostring(i2c_t *i2c, char *str, size_t len);

/* Error Handling */
int i2c_errno(i2c_t *i2c);
const char *i2c_errmsg(i2c_t *i2c);

/* struct i2c_msg from <linux/i2c.h>:

    struct i2c_msg {
    	__u16 addr;
    	__u16 flags;
    #define I2C_M_TEN		0x0010
    #define I2C_M_RD		0x0001
    #define I2C_M_STOP		0x8000
    #define I2C_M_NOSTART		0x4000
    #define I2C_M_REV_DIR_ADDR	0x2000
    #define I2C_M_IGNORE_NAK	0x1000
    #define I2C_M_NO_RD_ACK		0x0800
    #define I2C_M_RECV_LEN		0x0400
    	__u16 len;
    	__u8 *buf;
    };
*/
```

### DESCRIPTION

``` c
i2c_t *i2c_new(void);
```
Allocate an I2C handle.

Returns a valid handle on success, or NULL on failure.

------

``` c
int i2c_open(i2c_t *i2c, const char *device);
```
Open the `i2c-dev` device at the specified path (e.g. "/dev/i2c-1").

`i2c` should be a valid pointer to an allocated I2C handle structure.

Returns 0 on success, or a negative [I2C error code](#return-value) on failure.

------

``` c
int i2c_transfer(i2c_t *i2c, struct i2c_msg *msgs, size_t count);
```
Transfer `count` number of `struct i2c_msg` I2C messages.

`i2c` should be a valid pointer to an I2C handle opened with `i2c_open()`. `msgs` should be a pointer to an array of `struct i2c_msg` (defined in linux/i2c.h).

Each I2C message structure (see [above](#synopsis)) specifies the transfer of a consecutive number of bytes to a slave address. The slave address, message flags, buffer length, and pointer to a byte buffer should be specified in each message. The message flags specify whether the message is a read (I2C_M_RD) or write (0) transaction, as well as additional options selected by the bitwise OR of their bitmasks.

Returns 0 on success, or a negative [I2C error code](#return-value) on failure.

------

``` c
int i2c_close(i2c_t *i2c);
```
Close the `i2c-dev` device.

`i2c` should be a valid pointer to an I2C handle opened with `i2c_open()`.

Returns 0 on success, or a negative [I2C error code](#return-value) on failure.

------

``` c
void i2c_free(i2c_t *i2c);
```
Free an I2C handle.

------

``` c
int i2c_fd(i2c_t *i2c);
```
Return the file descriptor (for the underlying `i2c-dev` device) of the I2C handle.

`i2c` should be a valid pointer to an I2C handle opened with `i2c_open()`.

This function is a simple accessor to the I2C handle structure and always succeeds.

------

``` c
int i2c_tostring(i2c_t *i2c, char *str, size_t len);
```
Return a string representation of the I2C handle.

`i2c` should be a valid pointer to an I2C handle opened with `i2c_open()`.

This function behaves and returns like `snprintf()`.

------

``` c
int i2c_errno(i2c_t *i2c);
```
Return the libc errno of the last failure that occurred.

`i2c` should be a valid pointer to an I2C handle opened with `i2c_open()`.

------

``` c
const char *i2c_errmsg(i2c_t *i2c);
```
Return a human readable error message of the last failure that occurred.

`i2c` should be a valid pointer to an I2C handle opened with `i2c_open()`.

### RETURN VALUE

The periphery I2C functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `i2c_errno()` helper function. A human readable error message can be obtained with the `i2c_errmsg()` helper function.

| Error Code                | Description                       |
|---------------------------|-----------------------------------|
| `I2C_ERROR_ARG`           | Invalid arguments                 |
| `I2C_ERROR_OPEN`          | Opening I2C device                |
| `I2C_ERROR_QUERY`         | Querying I2C device attribtues    |
| `I2C_ERROR_NOT_SUPPORTED` | I2C not supported on this device  |
| `I2C_ERROR_TRANSFER`      | I2C transfer                      |
| `I2C_ERROR_CLOSE`         | Closing I2C device                |

### EXAMPLE

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

