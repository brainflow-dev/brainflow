### NAME

PWM wrapper functions for Linux userspace sysfs PWMs.

### SYNOPSIS

``` c
#include <periphery/pwm.h>

/* Primary Functions */
pwm_t *pwm_new(void);
int pwm_open(pwm_t *pwm, unsigned int chip, unsigned int channel);
int pwm_enable(pwm_t *pwm);
int pwm_disable(pwm_t *pwm);
int pwm_close(pwm_t *pwm);
void pwm_free(pwm_t *pwm);

/* Getters */
int pwm_get_enabled(pwm_t *pwm, bool *enabled);
int pwm_get_period_ns(pwm_t *pwm, uint64_t *period_ns);
int pwm_get_duty_cycle_ns(pwm_t *pwm, uint64_t *duty_cycle_ns);
int pwm_get_period(pwm_t *pwm, double *period);
int pwm_get_duty_cycle(pwm_t *pwm, double *duty_cycle);
int pwm_get_frequency(pwm_t *pwm, double *frequency);
int pwm_get_polarity(pwm_t *pwm, pwm_polarity_t *polarity);

/* Setters */
int pwm_set_enabled(pwm_t *pwm, bool enabled);
int pwm_set_period_ns(pwm_t *pwm, uint64_t period_ns);
int pwm_set_duty_cycle_ns(pwm_t *pwm, uint64_t duty_cycle_ns);
int pwm_set_period(pwm_t *pwm, double period);
int pwm_set_duty_cycle(pwm_t *pwm, double duty_cycle);
int pwm_set_frequency(pwm_t *pwm, double frequency);
int pwm_set_polarity(pwm_t *pwm, pwm_polarity_t polarity);

/* Miscellaneous */
unsigned int pwm_chip(pwm_t *pwm);
unsigned int pwm_channel(pwm_t *pwm);
int pwm_tostring(pwm_t *pwm, char *str, size_t len);

/* Error Handling */
int pwm_errno(pwm_t *pwm);
const char *pwm_errmsg(pwm_t *pwm);
```

### ENUMERATIONS

* `pwm_polarity_t`
    * `PWM_POLARITY_NORMAL`: Normal polarity
    * `PWM_POLARITY_INVERSED`: Inversed polarity

### DESCRIPTION

``` c
pwm_t *pwm_new(void);
```
Allocate a PWM handle.

Returns a valid handle on success, or NULL on failure.

------

``` c
int pwm_open(pwm_t *pwm, unsigned int chip, unsigned int channel);
```
Open the sysfs PWM with the specified chip and channel.

`pwm` should be a valid pointer to an allocated PWM handle structure.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_enable(pwm_t *pwm);
```
Enable the PWM output.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_disable(pwm_t *pwm);
```
Disable the PWM output.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_close(pwm_t *pwm);
```
Close the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
void pwm_free(pwm_t *pwm);
```
Free a PWM handle.

------

``` c
int pwm_get_enabled(pwm_t *pwm, bool *enabled);
```
Get the output state of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_get_period_ns(pwm_t *pwm, uint64_t *period_ns);
int pwm_get_duty_cycle_ns(pwm_t *pwm, uint64_t *duty_cycle_ns);
```
Get the period in nanoseconds or duty cycle in nanoseconds, respectively, of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_get_period(pwm_t *pwm, double *period);
int pwm_get_duty_cycle(pwm_t *pwm, double *duty_cycle);
int pwm_get_frequency(pwm_t *pwm, double *frequency);
```
Get the period in seconds, duty cycle as a ratio between 0.0 to 1.0, or frequency in Hz, respectively, of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_get_polarity(pwm_t *pwm, pwm_polarity_t *polarity);
```
Get the output polarity of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_set_enabled(pwm_t *pwm, bool enabled);
```
Set the output state of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_set_period_ns(pwm_t *pwm, uint64_t period_ns);
int pwm_set_duty_cycle_ns(pwm_t *pwm, uint64_t duty_cycle_ns);
```
Set the period in nanoseconds or duty cycle in nanoseconds, respectively, of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_set_period(pwm_t *pwm, double period);
int pwm_set_duty_cycle(pwm_t *pwm, double duty_cycle);
int pwm_set_frequency(pwm_t *pwm, double frequency);
```
Set the period in seconds, duty cycle as a ratio between 0.0 to 1.0, or frequency in Hz, respectively, of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
int pwm_set_polarity(pwm_t *pwm, pwm_polarity_t *polarity);
```
Set the output polarity of the PWM.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

Returns 0 on success, or a negative [PWM error code](#return-value) on failure.

------

``` c
unsigned int pwm_chip(pwm_t *pwm);
```
Return the chip number of the PWM handle.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

This function is a simple accessor to the PWM handle structure and always succeeds.

------

``` c
unsigned int pwm_channel(pwm_t *pwm);
```
Return the channel number of the PWM handle.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

This function is a simple accessor to the PWM handle structure and always succeeds.

------

``` c
int pwm_tostring(pwm_t *pwm, char *str, size_t len);
```
Return a string representation of the PWM handle.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

This function behaves and returns like `snprintf()`.

------

``` c
int pwm_errno(pwm_t *pwm);
```
Return the libc errno of the last failure that occurred.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

------

``` c
const char *pwm_errmsg(pwm_t *pwm);
```
Return a human readable error message of the last failure that occurred.

`pwm` should be a valid pointer to a PWM handle opened with `pwm_open()`.

### RETURN VALUE

The periphery PWM functions return 0 on success or one of the negative error codes below on failure.

The libc errno of the failure in an underlying libc library call can be obtained with the `pwm_errno()` helper function. A human readable error message can be obtained with the `pwm_errmsg()` helper function.

| Error Code            | Description                       |
|-----------------------|-----------------------------------|
| `PWM_ERROR_ARG`       | Invalid arguments                 |
| `PWM_ERROR_OPEN`      | Opening PWM                       |
| `PWM_ERROR_QUERY`     | Querying PWM attributes           |
| `PWM_ERROR_CONFIGURE` | Configuring PWM attributes        |
| `PWM_ERROR_CLOSE`     | Closing PWM                       |

### EXAMPLE

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

