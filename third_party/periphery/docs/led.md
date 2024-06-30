### NAME

LED wrapper functions for Linux userspace sysfs LEDs.

### SYNOPSIS

``` c
#include <periphery/led.h>

/* Primary Functions */
led_t *led_new(void);
int led_open(led_t *led, const char *name);
int led_read(led_t *led, bool *value);
int led_write(led_t *led, bool value);
int led_close(led_t *led);
void led_free(led_t *led);

/* Getters */
int led_get_brightness(led_t *led, unsigned int *brightness);
int led_get_max_brightness(led_t *led, unsigned int *max_brightness);

/* Setters */
int led_set_brightness(led_t *led, unsigned int brightness);

/* Miscellaneous */
int led_name(led_t *led, char *str, size_t len);
int led_tostring(led_t *led, char *str, size_t len);

/* Error Handling */
int led_errno(led_t *led);
const char *led_errmsg(led_t *led);
```

### DESCRIPTION

``` c
led_t *led_new(void);
```
Allocate an LED handle.

Returns a valid handle on success, or NULL on failure.

------

``` c
int led_open(led_t *led, const char *name);
```
Open the sysfs LED with the specified name.

`led` should be a valid pointer to an allocated LED handle structure.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
int led_read(led_t *led, bool *value);
```
Read the state of the LED into `value`, where `true` is non-zero brightness, and `false` is zero brightness.

`led` should be a valid pointer to an LED handle opened with `led_open()`. `value` should be a pointer to an allocated bool.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
int led_write(led_t *led, bool value);
```
Write the state of the LED to `value`, where `true` is max brightness, and `false` is zero brightness.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
int led_close(led_t *led);
```
Close the LED.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
void led_free(led_t *led);
```
Free an LED handle.

------

``` c
int led_get_brightness(led_t *led, unsigned int *brightness);
```
Get the brightness of the LED.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
int led_get_max_brightness(led_t *led, unsigned int *max_brightness);
```
Get the max brightness of the LED.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
int led_set_brightness(led_t *led, unsigned int brightness);
```
Set the brightness of the LED.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
int led_name(led_t *led, char *str, size_t len);
```
Return the name of the sysfs LED.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

Returns 0 on success, or a negative [LED error code](#return-value) on failure.

------

``` c
int led_tostring(led_t *led, char *str, size_t len);
```
Return a string representation of the LED handle.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

This function behaves and returns like `snprintf()`.

------

``` c
int led_errno(led_t *led);
```
Return the libc errno of the last failure that occurred.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

------

``` c
const char *led_errmsg(led_t *led);
```
Return a human readable error message of the last failure that occurred.

`led` should be a valid pointer to an LED handle opened with `led_open()`.

### RETURN VALUE

The periphery LED functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `led_errno()` helper function. A human readable error message can be obtained with the `led_errmsg()` helper function.

| Error Code            | Description                       |
|-----------------------|-----------------------------------|
| `LED_ERROR_ARG`       | Invalid arguments                 |
| `LED_ERROR_OPEN`      | Opening LED                       |
| `LED_ERROR_QUERY`     | Querying LED attributes           |
| `LED_ERROR_IO`        | Reading/writing LED brightness    |
| `LED_ERROR_CLOSE`     | Closing LED                       |

### EXAMPLE

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

